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

#include "FSTAssembly.h"
#include "Dictionary.h"
#include "../Data/Utils.h"
#include "../Data/Data.h"

using namespace std;
using namespace Ear;

CFSTAssembly::CFSTAssembly()
{

}

CFSTAssembly::~CFSTAssembly()
{
    /// removing and deleting all transitions from the multimap
    multimap<unsigned int, EAR_FST_Trn*>::iterator it;
    for (it = m_fst.begin(); it != m_fst.end(); it++) {
        delete it->second;
    }

    /// clearing the multimap of pointers
    m_fst.clear();
}

int CFSTAssembly::assembly(CHTKAcousticModel *_model, CDictionary *_dict)
{
    m_model = _model;
    m_dict = _dict;

    /// get the linked list of the dictionary
    DictItem *word = _dict->getDict();

    char *modelName = NULL;
    unsigned int iLast;
    HTK_Data *hmm = NULL;
    short first = 1;
    unsigned int iEndState = 0;

    /// create state manager
    StateManager States;

    /// reserve number for end state ... later we will mark this state as end state
    iEndState = States.getNewNumber();

    /// go through all dictionary items/events
    while (word != NULL) {
        iLast = START_STATE; ///< last state used, default value the start state
        modelName = parse(word->models); ///< get model name of the current event
        first = 1; ///< we need to remember when we have used the first transition for current event.

        /// there is model name
        while (modelName != NULL) {
            //get needed data
            hmm = _model->searchForData(modelName);
            modelName = parseNext(); //get next model end check if not end
            if (hmm == NULL) return EAR_FAIL; //missing model in acoustic parameters

            //ready states for fst
            if (modelName == NULL) States.ready(iLast, iEndState, hmm->is);
            else iLast = States.ready(iLast, UNDEF_STATE, hmm->is);

            //create fst with states
            if (first) {
                createFST(States, hmm, word);
                first = 0;
            }
            else createFST(States, hmm, NULL);
        }

        //next word
        word = word->next;
    }

    //mark end state as end state ... basicaly connect end state to abstract end state
    connect(iEndState, END_STATE, EPS_SYM, EPS_SYM, 0.0);

    //create loop from end state to the beginning
    connect(iEndState, START_STATE, EPS_SYM, EPS_SYM, 0.0);

    return EAR_SUCCESS;
}

void CFSTAssembly::createFST(StateManager &_states, HTK_Data *_hmm, DictItem *_word)
{
    //declare
    EAR_FST_Trn *item = NULL;
    HTK_Trans *t = _hmm->tr;
    unsigned int i, j;

    //construct transitions
    for (i = 0; i < t->size; i++) {
        for (j = 0; j < t->size; j++) {
            if (t->p[i][j] != LOG_ZERO) {
                item = new EAR_FST_Trn;
                item->iStart = _states[i];
                item->iEnd = _states[j];

                if (_hmm != NULL && _hmm->s[j] != NULL) item->iIn = _hmm->s[j]->id;
                else item->iIn = EPS_SYM;

                if (i == 0 && _word != NULL) item->iOut = _word->id;
                else item->iOut = EPS_SYM;

                if (i == 0 && _word != NULL) item->fWeight = t->p[i][j] + _word->prob;
                else item->fWeight = t->p[i][j];

                m_fst.insert(pair<unsigned int, EAR_FST_Trn*>(item->iStart, item));
            }
        }
    }
}

void CFSTAssembly::connect(unsigned int _s, unsigned int _e, unsigned int _i, unsigned int _o, float _w)
{
    EAR_FST_Trn *item = NULL;
    item = new EAR_FST_Trn;
    item->iStart = _s;
    item->iEnd = _e;
    item->iIn = _i;
    item->iOut = _o;
    item->fWeight = _w;

    m_fst.insert(pair<unsigned int, EAR_FST_Trn*>(item->iStart, item));
}

int CFSTAssembly::writeFST(const char *_szFstName, const char *_iSymName, const char *_oSymName)
{
    if (writeFSTFile(_szFstName) == EAR_FAIL) return EAR_FAIL;
    if (writeISymFile(_iSymName) == EAR_FAIL) return EAR_FAIL;
    if (writeOSymFile(_oSymName) == EAR_FAIL) return EAR_FAIL;

    return EAR_SUCCESS;
}

int CFSTAssembly::writeFSTFile(const char *_szFileName)
{
    FILE *pf = NULL;
    multimap<unsigned int, EAR_FST_Trn*>::iterator it;
    EAR_FST_Trn *t = NULL;

    pf = fopen(_szFileName, "w");
    if (pf == NULL) return EAR_FAIL;

    for (it = m_fst.begin(); it != m_fst.end(); it++) {
        t = it->second;
        if (t->iEnd == END_STATE) fprintf(pf, "%d\t%f\n", t->iStart, 0.0); //fprintf(pf, "%d\t%d\t%d\t%d\t%f\n", t->iStart, end, t->iIn, t->iOut, t->fWeight);
        else fprintf(pf, "%d\t%d\t%d\t%d\t%f\n", t->iStart, t->iEnd, t->iIn, t->iOut, t->fWeight);
    }

    fclose(pf);

    return EAR_SUCCESS;
}

int CFSTAssembly::writeISymFile(const char *_szFileName)
{
    FILE *pf = NULL;
    map<unsigned int, char*> items;
    map<unsigned int, char*>::iterator it;
    HTK_State *state = m_model->getHMMInfo()->s;
    char *name;
    char temp[100];

    pf = fopen(_szFileName, "w");
    if (pf == NULL) return EAR_FAIL;

    fprintf(pf, "<eps>\t0\n");

    while (state != NULL) {
        if (state->name == NULL) {
            sprintf(temp, "STATE-%d", state->id);
            name = cloneString(temp);
        }
        else {
            name = cloneString(state->name);
        }

        items[state->id] = name;

        state = state->next;
    }

    for (it = items.begin(); it != items.end(); it++) {
        fprintf(pf, "%s\t%d\n", it->second, it->first);
        delete[] it->second;
    }

    fclose(pf);
    items.clear();

    return EAR_SUCCESS;
}

int CFSTAssembly::writeOSymFile(const char *_szFileName)
{
    FILE *pf = NULL;
    map<unsigned int, char*> items;
    map<unsigned int, char*>::iterator it;
    DictItem *w = m_dict->getDict();

    pf = fopen(_szFileName, "w");
    if (pf == NULL) return EAR_FAIL;

    fprintf(pf, "<eps>\t0\n");

    while (w != NULL) {
        items[w->id] = w->word;
        w = w->next;
    }

    for (it = items.begin(); it != items.end(); it++) {
        fprintf(pf, "%s\t%d\n", it->second, it->first);
    }

    fclose(pf);

    return EAR_SUCCESS;
}

int CFSTAssembly::writeBin(const char *_szOut, const char *_szOutIndex)
{
    FILE *pf = NULL;
    unsigned int i;
    unsigned int size;
    multimap<unsigned int, EAR_FST_Trn*>::iterator it;
    EAR_FST_Trn *t;

    pf = fopen(_szOut, "wb");
    if (pf == NULL) return EAR_FAIL;

    EAR_AM_Info *model = m_model->getAcousticModel();
    if (model == NULL) return EAR_FAIL;

    //write vector size
    fwrite(&model->iVectorSize, sizeof (unsigned short), 1, pf);

    //write number of states
    fwrite(&model->iNumberOfStates, sizeof (unsigned int), 1, pf);

    //write number of pdfs
    fwrite(&model->iNumberOfPdfs, sizeof (unsigned int), 1, pf);

    //write number of pdfs on one state
    fwrite(&model->iPdfsOnState, sizeof (unsigned int), 1, pf);

    //go through states and write
    for (i = 0; i < model->iNumberOfStates; i++)
        fwrite(model->States[i], sizeof (unsigned int), model->iPdfsOnState, pf);

    //go through pdfs and write
    for (i = 0; i < model->iNumberOfPdfs; i++) {
        fwrite(model->Pdfs[i].fVar, sizeof (float), model->iVectorSize, pf);
        fwrite(model->Pdfs[i].fMean, sizeof (float), model->iVectorSize, pf);
        fwrite(&model->Pdfs[i].fgconst, sizeof (float), 1, pf);
        fwrite(&model->Pdfs[i].fWeight, sizeof (float), 1, pf);
    }

    //write number of transitions in fst
    size = m_fst.size();
    fwrite(&size, sizeof (unsigned int), 1, pf);

    //write all transitions from fst
    for (it = m_fst.begin(); it != m_fst.end(); it++) {
        t = it->second;
        fwrite(&t->iStart, sizeof (unsigned int), 1, pf);
        fwrite(&t->iEnd, sizeof (unsigned int), 1, pf);
        fwrite(&t->iIn, sizeof (unsigned int), 1, pf);
        fwrite(&t->iOut, sizeof (unsigned int), 1, pf);
        fwrite(&t->fWeight, sizeof (float), 1, pf);
    }

    fclose(pf);

    for (unsigned int i = 0; i < model->iNumberOfStates; i++) {
        delete[] model->States[i];
    }
    delete[] model->States;

    for (unsigned int i = 0; i < model->iNumberOfPdfs; i++) {
        delete[] model->Pdfs[i].fVar;
        delete[] model->Pdfs[i].fMean;
    }

    delete[] model->Pdfs;
    delete model;

    writeOSymFile(_szOutIndex);

    return EAR_SUCCESS;
}

StateManager::StateManager()
{
    StateArray = NULL;
    m_count = START_STATE;
    m_size = 0;
}

StateManager::~StateManager()
{
    if (StateArray != NULL) delete[] StateArray;
}

unsigned int &StateManager::operator[](unsigned int _i)
{
    return StateArray[_i];
}

void StateManager::size(unsigned int _size)
{
    if (_size > m_size) {
        delete[] StateArray;
        StateArray = new unsigned int[_size];
        m_size = _size;
    }
}

unsigned int StateManager::ready(unsigned int _start, unsigned int _last, unsigned int _number)
{
    unsigned int i;

    size(_number);
    for (i = 0; i < _number; i++) {
        if (i == 0) {
            StateArray[i] = _start;
            continue;
        }

        if (i == _number - 1) {
            if (_last == UNDEF_STATE) StateArray[i] = getNewNumber();
            else StateArray[i] = _last;
            continue;
        }

        StateArray[i] = getNewNumber();
    }

    return StateArray[i - 1];
}

unsigned int StateManager::getNewNumber()
{
    m_count++;
    return m_count;
}
