/*
 * Copyright (c) 2017 Technical University of Košice (author: Martin Lojka)
 *
 * This file is part of EAR-TUKE.
 *
 * EAR-TUKE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EAR-TUKE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EAR-TUKE. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <math.h>

#include "HTKAcousticModel.h"
#include "../Data/Utils.h"

using namespace std;
using namespace Ear;

CHTKAcousticModel::CHTKAcousticModel()
{
    //ready model parameter holder
    m_HTK_info.idata = 0;
    m_HTK_info.iPdfs = 0;
    m_HTK_info.iStates = 0;
    m_HTK_info.iTrs = 0;

    m_HTK_info.data = NULL;
    m_HTK_info.pdf = NULL;
    m_HTK_info.s = NULL;
    m_HTK_info.trs = NULL;
}

CHTKAcousticModel::~CHTKAcousticModel()
{
    HTK_Trans *t = NULL;
    HTK_Pdf *p = NULL;
    HTK_State *s = NULL;
    HTK_Data *d = NULL;

    HTK_Trans *t_ = m_HTK_info.trs;
    HTK_Pdf *p_ = m_HTK_info.pdf;
    HTK_State *s_ = m_HTK_info.s;
    HTK_Data *d_ = m_HTK_info.data;

    unsigned int i;

    /// go though each individual lists and remove, delete them from memory
    while (t_ != NULL) {
        t = t_;
        t_ = t->next;

        for (i = 0; i < t->size; i++)
            delete[] t->p[i];

        delete[] t->p;
        delete[] t->name;
        delete t;
    }

    while (p_ != NULL) {
        p = p_;
        p_ = p->next;
        delete[] p->mean;
        delete[] p->name;
        delete[] p->var;
        delete p;
    }

    while (s_ != NULL) {
        s = s_;
        s_ = s->next;
        //for(i=0; i< s->ipdfs; i++)
        //delete s->pdfs[i];

        delete[] s->pdfs;
        delete[] s->name;
        delete s;
    }

    while (d_ != NULL) {
        d = d_;
        d_ = d->next;
        //for(i=0; i< d->is; i++)
        //delete[] d->s[i];

        delete[] d->name;
        delete[] d->s;
        //delete d->tr;
        delete d;
    }
}

int CHTKAcousticModel::loadModelFrom(char *_szFileName)
{
    /// open the file with appropriate segmentation tokens. The HTK format uses tags in <> brackets and quotes.
    if (m_file.open(_szFileName, m_file.READ, "<>\" ") != EAR_SUCCESS) return EAR_FAIL;
    /// read header with vector size and type of acoustic model information
    if (readHMMHeader() == EAR_FAIL) {
        printf("Error reading HMM file header\n");
        m_file.close();
        return EAR_FAIL;
    }
    /// read the rest, the whole acoutic model definition
    if (readHMMInfo() == EAR_FAIL) {
        printf("Error reading HMM file\n");
        m_file.close();
        return EAR_FAIL;
    }
    /// close and success
    m_file.close();
    return EAR_SUCCESS;
}

HTK_Info *CHTKAcousticModel::getHMMInfo()
{
    /// return HTK structure of the acoustic model
    return &m_HTK_info;
}

int CHTKAcousticModel::readHMMHeader()
{
    char *buf = NULL;
    /// VECSIZE the size of the required feature vector
    if (m_file.skipTo("VECSIZE") == NULL) return EAR_FAIL;

    /// read token
    buf = m_file.read();
    m_HTK_info.iVectorSize = atoi(buf);

    /// skip one token and read next one (the type of the acoustic model)
    buf = m_file.read();
    buf = m_file.read();
    strcpy(m_HTK_info.szType, buf);

    return EAR_SUCCESS;
}

int CHTKAcousticModel::readHMMInfo()
{
    char *buf = NULL;
    char *name = NULL;

    /// read token
    buf = m_file.read();

    while (buf != NULL) {
        /// the token is matrix macro definition
        if (m_file.read("~t")) {
            macroTransp();
        }
            /// the token is state macro definition
        else if (m_file.read("~s")) {
            macroState();
        }
            /// the token is model macro definition
        else if (m_file.read("~h")) {
            macroModel();
        }
            /// read the next token when finished with macros
        else buf = m_file.read(); //not sure
        //m_file.read();
    }

    return EAR_SUCCESS;
}

int CHTKAcousticModel::macroTransp()
{
    char *buf = NULL;
    char *name = NULL;

    /// name of the macro
    buf = m_file.read();
    name = cloneString(buf);
    /// consume the transposition matrix
    if (saveTransp(name) == NULL) return EAR_FAIL;

    return EAR_SUCCESS;
}

int CHTKAcousticModel::macroState()
{
    char *buf = NULL;
    char *name = NULL;

    /// name of the macro
    buf = m_file.read();
    name = cloneString(buf);
    /// consume the state definition
    if (saveState(name) == NULL) return EAR_FAIL;

    return EAR_SUCCESS;
}

int CHTKAcousticModel::macroModel()
{
    char *buf = NULL;
    char *name = NULL;

    /// name of the model macro
    buf = m_file.read();
    name = cloneString(buf);
    /// consume the model definition
    if (saveModel(name) == NULL) return EAR_FAIL;
    return EAR_SUCCESS;
}

HTK_Trans *CHTKAcousticModel::searchForTransp(char *_name)
{
    /// simple search for name in the hash map and return
    if (_name == NULL) return NULL;
    map<char*, HTK_Trans*, cmp_str>::iterator it;
    it = mapTrs.begin();
    it = mapTrs.find(_name);
    if (it != mapTrs.end()) return it->second;

    return NULL;
}

HTK_Data *CHTKAcousticModel::searchForData(char *_name)
{
    /// simple search for name in the hash map and return
    if (_name == NULL) return NULL;
    map<char*, HTK_Data*, cmp_str>::iterator it;
    it = mapData.begin();
    it = mapData.find(_name);
    if (it != mapData.end()) return it->second;

    return NULL;
}

HTK_State *CHTKAcousticModel::searchForStates(char *_name)
{
    /// simple search for name in the hash map and return
    if (_name == NULL) return NULL;
    map<char*, HTK_State*, cmp_str>::iterator it;
    it = mapStates.begin();
    it = mapStates.find(_name);
    if (it != mapStates.end()) return it->second;

    return NULL;
}

void CHTKAcousticModel::add(HTK_Trans *_item)
{
    /// add to the top level structure and increase the id by one
    if (m_HTK_info.trs) {
        _item->next = m_HTK_info.trs;
        _item->id = m_HTK_info.trs->id + 1;
    }
    else {
        _item->id = 1;
    }
    m_HTK_info.trs = _item;
    m_HTK_info.iTrs++;

    /// if named entity save for later search
    if (_item->name != NULL) mapTrs[_item->name] = _item;
}

void CHTKAcousticModel::add(HTK_State *_item)
{
    /// add to the top level structure and increase the id by one
    if (m_HTK_info.s) {
        _item->next = m_HTK_info.s;
        _item->id = m_HTK_info.s->id + 1;
    }
    else {
        _item->id = 1;
    }
    m_HTK_info.s = _item;
    m_HTK_info.iStates++;

    /// if named entity save for later search
    if (_item->name != NULL) mapStates[_item->name] = _item;
}

void CHTKAcousticModel::add(HTK_Data *_item)
{
    /// add to the top level structure and increase the id by one
    if (m_HTK_info.data) {
        _item->next = m_HTK_info.data;
        _item->id = m_HTK_info.data->id + 1;
    }
    else {
        _item->id = 1;
    }
    m_HTK_info.data = _item;
    m_HTK_info.idata++;

    /// if named entity save for later search
    if (_item->name != NULL) mapData[_item->name] = _item;
}

void CHTKAcousticModel::add(HTK_Pdf *_item)
{
    /// add to the top level structure and increase the id by one
    if (m_HTK_info.pdf) {
        _item->next = m_HTK_info.pdf;
        _item->id = m_HTK_info.pdf->id + 1;
    }
    else {
        _item->id = 1;
    }
    m_HTK_info.pdf = _item;
    m_HTK_info.iPdfs++;
}

HTK_Trans *CHTKAcousticModel::saveTransp(char *_name)
{
    char *buf;
    /// the matrix begins with TRANSP tag
    if (m_file.read("TRANSP") == NULL) return NULL;

    HTK_Trans *Item = new HTK_Trans;
    Item->next = NULL;

    /// name
    Item->name = _name;

    /// get size of the matrix
    buf = m_file.read();
    Item->size = atoi(buf);

    /// read the matrix and transform the values to logarithm
    Item->p = read2Matrix(Item->size, Item->size, 1);

    /// add the new matrix to the toplevel structure
    add(Item);

    /// also return to calling function to add it to parent structure
    return Item;
}

HTK_State *CHTKAcousticModel::saveState(char *_name)
{
    char *buf;
    unsigned int nummixes = 1, i; /// initializing to one

    /// get number of pdf in for this state
    /// this is not included when the number of pdfs is one
    if (m_file.read("NUMMIXES") != NULL) {
        buf = m_file.read();
        nummixes = atoi(buf);
    }

    HTK_State *Item = new HTK_State;
    Item->next = NULL;

    /// name
    Item->name = _name;
    Item->ipdfs = nummixes;

    /// initialize the pdfs array
    Item->pdfs = new HTK_Pdf*[Item->ipdfs];

    /// null the array and start to consume one by one pdf at a time
    for (i = 0; i < Item->ipdfs; i++) {
        Item->pdfs[i] = NULL;
        Item->pdfs[i] = savePdf(NULL);
        //if(Item->pdfs[i] == NULL) return NULL;
    }

    //read
    /*	while(m_file.read("MIXTURE") != NULL)
            {
                    buf = m_file.read(); i = atoi(buf);
                    Item->pdfs[i-1] = savePdf(NULL);
            }*/

    /// add the new state into top level structure
    add(Item);

    /// return to the calling function to include the state into parent structure
    return Item;
}

HTK_Pdf *CHTKAcousticModel::savePdf(char *_name)
{
    char *buf;
    unsigned int size;
    float weight = 1.0; /// initializing the weight to one

    /// read the pdf's weight. This is not included if the weight is one
    if (m_file.read("MIXTURE") != NULL) {
        buf = m_file.read(); ///< this is index of the pdf, do not need it
        buf = m_file.read(); ///< this is the weight
        weight = atoi(buf);
    }

    HTK_Pdf *Item = new HTK_Pdf;
    Item->next = NULL;

    /// name
    Item->name = _name;
    Item->weight = weight;

    /// consume mean of the PDF as vector
    if (m_file.read("MEAN") == NULL) {
        delete Item;
        return NULL;
    }
    buf = m_file.read();
    size = atoi(buf);
    Item->mean = readVector(size);

    /// consume variance of the PDF as vector
    if (m_file.read("VARIANCE") == NULL) {
        delete Item;
        return NULL;
    }
    buf = m_file.read();
    size = atoi(buf);
    Item->var = readVector(size);
    invert(Item->var, size); ///< do not forget to invert the variance now saving time in runtime computation.

    /// precomputed value for this PDF.
    if (m_file.read("GCONST") == NULL) {
        delete Item;
        return NULL;
    }
    buf = m_file.read();
    Item->gconst = atof(buf);

    /// include the PDF to the toplevel structure
    add(Item);

    /// return to the calling function to include it into parent structure
    return Item;
}

HTK_Data *CHTKAcousticModel::saveModel(char *_name)
{
    /// model begins with tag BEGINHMM.. so this is required to be there
    if (m_file.read("BEGINHMM") == NULL) return NULL;

    char *buf;
    unsigned int i;
    HTK_Data *Item = new HTK_Data;
    Item->next = NULL;

    /// name
    Item->name = _name;

    /// get number of states
    buf = m_file.read(); ///< this should be tag <NUMSTATES>
    buf = m_file.read(); ///< this is the number of states
    Item->is = atoi(buf);

    /// allocate
    Item->s = new HTK_State*[Item->is];

    /// null array
    for (i = 0; i < Item->is; i++) {
        Item->s[i] = NULL;
    }

    /// read
    while (m_file.read("STATE") != NULL) {
        /// get the state number
        buf = m_file.read();
        i = atoi(buf);
        /// if there is macro of the state, we have already read the state, search for it
        if (m_file.read("~s") != NULL) {
            buf = m_file.read();
            Item->s[i - 1] = searchForStates(buf);
        }
            /// otherwise there is whole definition to read, call <i>saveState</i> to consume the state
            /// inlude output to this new structure
        else {
            Item->s[i - 1] = saveState(NULL);
            if (Item->s[i - 1] == NULL) return NULL;
        }
    }

    /// read transition matrix
    /// if defined as macro, we have already read it, search for it by name
    if (m_file.read("~t") != NULL) {
        buf = m_file.read();
        Item->tr = searchForTransp(buf);
    }
        /// otherwise consume the matrix there and add to this new structure
    else {
        Item->tr = saveTransp(NULL);
        if (Item->tr == NULL) return NULL;
    }

    /// there should be end tag at the end of the model definition
    m_file.read("ENDHMM");

    /// include the model into toplevel structure
    add(Item);

    /// also return to the calling function to include it into parent structure
    return Item;
}

float **CHTKAcousticModel::read2Matrix(unsigned int _x, unsigned int _y, unsigned int _log)
{
    char *buf;
    float **f = NULL;
    unsigned int i, j;
    float l;

    f = new float*[_x];
    for (i = 0; i < _x; i++) {
        f[i] = new float[_y];
        for (j = 0; j < _y; j++) {
            /// read the value
            buf = m_file.read();
            /// transform to float
            l = atof(buf);
            /// compute log if needed
            if (_log) {
                f[i][j] = (l != 0.0) ? (float) log(l) : LOG_ZERO; ///< compute log, if zero return LOG_ZERO defined
                ///< all values in the decoding process are negative, so we do not need to remember the sign, removing it
                if (f[i][j] != LOG_ZERO && f[i][j] != 0.0) f[i][j] *= (-1.0);
            }
            else {
                f[i][j] = l;
            }
        }
    }

    return f;
}

float *CHTKAcousticModel::readVector(unsigned int _x)
{
    char *buf;
    unsigned int i;
    float *f = NULL;

    f = new float[_x];
    for (i = 0; i < _x; i++) {
        buf = m_file.read(); ///< read the value
        f[i] = atof(buf); ///< transform to float
    }

    return f;
}

void CHTKAcousticModel::invert(float *_f, unsigned int _size)
{
    unsigned int i;
    for (i = 0; i < _size; i++) _f[i] = 1.0 / _f[i];
}

EAR_AM_Info *CHTKAcousticModel::getAcousticModel()
{
    //declare
    EAR_AM_Info *out = NULL;
    HTK_State *s = NULL;
    HTK_Pdf *p = NULL;
    unsigned int i, j;
    unsigned int pdfsOnState = 0;

    /// Compute the maximum number of PDFs for state
    /// so we can allocate the unified structure.
    HTK_State *temp = m_HTK_info.s;
    while (temp != NULL) {
        if (pdfsOnState < temp->ipdfs) {
            pdfsOnState = temp->ipdfs;
        }
        temp = temp->next;
    }

    /// allocate
    out = new EAR_AM_Info;

    /// Add basic model definitions
    out->iVectorSize = m_HTK_info.iVectorSize;
    out->iNumberOfPdfs = m_HTK_info.iPdfs;
    out->iNumberOfStates = m_HTK_info.iStates;
    out->iPdfsOnState = pdfsOnState;

    /// allocating states and pdfs
    out->States = new unsigned int*[out->iNumberOfStates];
    for (i = 0; i < out->iNumberOfStates; i++)
        out->States[i] = new unsigned int[out->iPdfsOnState];
    out->Pdfs = new EAR_AM_Pdf[out->iNumberOfPdfs];

    /// null state array
    for (i = 0; i < out->iNumberOfStates; i++)
        for (j = 0; j < out->iPdfsOnState; j++)
            out->States[i][j] = NONE;

    /// fill pdf array
    /// using previous assigned ids as ids in new arrays
    p = m_HTK_info.pdf;
    while (p != NULL) {
        out->Pdfs[p->id - 1].fVar = cloneVector(p->var, out->iVectorSize);
        out->Pdfs[p->id - 1].fMean = cloneVector(p->mean, out->iVectorSize);
        out->Pdfs[p->id - 1].fgconst = p->gconst;
        out->Pdfs[p->id - 1].fWeight = p->weight;

        /// next pdf
        p = p->next;
    }

    /// fill the state array
    /// using previously assigned ids as ids in new arrays
    s = m_HTK_info.s;
    while (s != NULL) {
        for (i = 0; i < s->ipdfs; i++) {
            p = s->pdfs[i];
            /// HTK at some point in training process tend to remove some PDF definitions
            /// so thus, so we need to set those as NONE.
            if (p == NULL) out->States[s->id - 1][i] = NONE;
                /// save the id of the pdf
            else out->States[s->id - 1][i] = p->id;
        }

        /// next state
        s = s->next;
    }

    return out;
}
