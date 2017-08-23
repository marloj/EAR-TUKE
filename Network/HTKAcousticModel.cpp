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
	HTK_Pdf   *p = NULL;
	HTK_State *s = NULL;
	HTK_Data  *d = NULL;

	HTK_Trans *t_ = m_HTK_info.trs;
	HTK_Pdf   *p_ = m_HTK_info.pdf;
	HTK_State *s_ = m_HTK_info.s;
	HTK_Data  *d_ = m_HTK_info.data;

	unsigned int i;

	while(t_ != NULL)
	{
		t = t_;
		t_ = t->next;

		for(i=0; i< t->size; i++)
			delete[] t->p[i];

		delete[] t->p;
		delete[] t->name;
		delete t;
	}

	while(p_ != NULL)
	{
		p = p_;
		p_ = p->next;
		delete[] p->mean;
		delete[] p->name;
		delete[] p->var;
		delete p;
	}

	while(s_ != NULL)
	{
		s = s_;
		s_ = s->next;
		//for(i=0; i< s->ipdfs; i++)
			//delete s->pdfs[i];

		delete[] s->pdfs;
		delete[] s->name;
		delete s;
	}

	while(d_ != NULL)
	{
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
	if(m_file.open(_szFileName, m_file.READ, "<>\" ") != EAR_SUCCESS) return EAR_FAIL;
	if(readHMMHeader() == EAR_FAIL){ fprintf(stderr, "Error reading HMM file header\n"); m_file.close(); return EAR_FAIL; }
	if(readHMMInfo() == EAR_FAIL){ fprintf(stderr, "Error reading HMM file\n"); m_file.close(); return EAR_FAIL; }
	m_file.close();
}

HTK_Info *CHTKAcousticModel::getHMMInfo()
{
	return &m_HTK_info;
}

int CHTKAcousticModel::readHMMHeader()
{
	char *buf = NULL;
	if(m_file.skipTo("VECSIZE") == NULL) return EAR_FAIL;

	buf = m_file.read();
	m_HTK_info.iVectorSize = atoi(buf);

	buf = m_file.read();
	buf = m_file.read();
	strcpy(m_HTK_info.szType,buf);

	return EAR_SUCCESS;
}

int CHTKAcousticModel::readHMMInfo()
{
	char *buf = NULL;
	char *name = NULL;
	buf = m_file.read();

	while(buf != NULL)
	{
		if(m_file.read("~t")) {macroTransp();	  }
		else if(m_file.read("~s")) {macroState(); }
		else if(m_file.read("~h")) {macroModel(); }
		else buf = m_file.read(); //not sure
		//m_file.read();
	}

	return EAR_SUCCESS;
}

int CHTKAcousticModel::macroTransp()
{
	char *buf = NULL;
	char *name = NULL;

	buf = m_file.read(); name = cloneString(buf);
	if(saveTransp(name) == NULL) return EAR_FAIL;

	return EAR_SUCCESS;
}

int CHTKAcousticModel::macroState()
{
	char *buf = NULL;
	char *name = NULL;

	buf = m_file.read(); name = cloneString(buf);
	if(saveState(name) == NULL) return EAR_FAIL;

	return EAR_SUCCESS;
}

int CHTKAcousticModel::macroModel()
{
	char *buf = NULL;
	char *name = NULL;

	buf = m_file.read(); name = cloneString(buf);
	if(saveModel(name) == NULL) return EAR_FAIL;
	return EAR_SUCCESS;
}

HTK_Trans *CHTKAcousticModel::searchForTransp(char *_name)
{
	if(_name == NULL) return NULL;
	map<char*, HTK_Trans*, cmp_str>::iterator it;
	it = mapTrs.begin();
	it = mapTrs.find(_name);
	if(it != mapTrs.end()) return it->second;

	return NULL;
}

HTK_Data *CHTKAcousticModel::searchForData(char *_name)
{
	if(_name == NULL) return NULL;
	map<char*, HTK_Data*, cmp_str>::iterator it;
	it = mapData.begin();
	it = mapData.find(_name);
	if(it != mapData.end()) return it->second;

	return NULL;
}

HTK_State *CHTKAcousticModel::searchForStates(char *_name)
{
	if(_name == NULL) return NULL;
	map<char*, HTK_State*, cmp_str>::iterator it;
	it = mapStates.begin();
	it = mapStates.find(_name);
	if(it != mapStates.end()) return it->second;

	return NULL;
}

void CHTKAcousticModel::add(HTK_Trans *_item)
{
	if(m_HTK_info.trs) {_item->next = m_HTK_info.trs; _item->id = m_HTK_info.trs->id + 1;}
	else {_item->id = 1;}
	m_HTK_info.trs = _item;
	m_HTK_info.iTrs++;

	if(_item->name != NULL) mapTrs[_item->name] = _item;
}

void CHTKAcousticModel::add(HTK_State *_item)
{
	if(m_HTK_info.s) {_item->next = m_HTK_info.s; _item->id = m_HTK_info.s->id + 1;}
	else {_item->id = 1;}
	m_HTK_info.s = _item;
	m_HTK_info.iStates++;

	if(_item->name != NULL) mapStates[_item->name] = _item;
}

void CHTKAcousticModel::add(HTK_Data *_item)
{
	if(m_HTK_info.data) {_item->next = m_HTK_info.data; _item->id = m_HTK_info.data->id + 1;}
	else {_item->id = 1;}
	m_HTK_info.data = _item;
	m_HTK_info.idata++;

	if(_item->name != NULL) mapData[_item->name] = _item;
}

void CHTKAcousticModel::add(HTK_Pdf *_item)
{
	if(m_HTK_info.pdf) {_item->next = m_HTK_info.pdf; _item->id = m_HTK_info.pdf->id + 1;}
	else {_item->id = 1;}
	m_HTK_info.pdf = _item;
	m_HTK_info.iPdfs++;
}

HTK_Trans *CHTKAcousticModel::saveTransp(char *_name)
{
	//declare
	char *buf;
	if(m_file.read("TRANSP") == NULL) return NULL;

	HTK_Trans *Item = new HTK_Trans;
	Item->next = NULL;

	//save name
	Item->name = _name;

	//get size
	buf = m_file.read();
	Item->size = atoi(buf);

	//read matrix
	Item->p = read2Matrix(Item->size, Item->size, 1);

	//add new record
	add(Item);

	return Item;
}

HTK_State *CHTKAcousticModel::saveState(char *_name)
{
	//declare
	char *buf;
	unsigned int nummixes = 1, i;

	if(m_file.read("NUMMIXES") != NULL){
		//get number of mixtures
		buf = m_file.read();
		nummixes = atoi(buf);
	}
	
	HTK_State *Item = new HTK_State;
	Item->next = NULL;

	//save name
	Item->name = _name;
	Item->ipdfs=nummixes;

	//allocate
	Item->pdfs = new HTK_Pdf*[Item->ipdfs];

	//null array
	for(i=0; i<Item->ipdfs; i++) { 
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

	add(Item);

	return Item;
}

HTK_Pdf *CHTKAcousticModel::savePdf(char *_name)
{
	//declare
	char *buf;
	unsigned int size;
	float weight = 1.0;

	if(m_file.read("MIXTURE") != NULL){
		buf = m_file.read();
		buf = m_file.read();
		weight = atoi(buf);
	}

	HTK_Pdf *Item = new HTK_Pdf;
	Item->next = NULL;

	//name
	Item->name = _name;
	Item->weight = weight;

	//mean
	if(m_file.read("MEAN") == NULL) { delete Item; return NULL; } 
	buf = m_file.read();
	size = atoi(buf);
	Item->mean = readVector(size);

	//variance
	if(m_file.read("VARIANCE") == NULL){ delete Item; return NULL; }
	buf = m_file.read();
	size = atoi(buf);
	Item->var = readVector(size);
	invert(Item->var, size);

	//gconst
	if(m_file.read("GCONST") == NULL){ delete Item; return NULL; }
	buf = m_file.read();
	Item->gconst = atof(buf);

	add(Item);

	return Item;
}

HTK_Data *CHTKAcousticModel::saveModel(char *_name)
{
	if(m_file.read("BEGINHMM") == NULL) return NULL;

	//declare
	char *buf;
	unsigned int i;
	HTK_Data *Item = new HTK_Data;
	Item->next = NULL;

	//name
	Item->name = _name;

	//get number of states
	buf = m_file.read();
	buf = m_file.read();
	Item->is = atoi(buf);

	//allocate
	Item->s = new HTK_State*[Item->is];

	//null array
	for(i=0; i<Item->is; i++) {Item->s[i] = NULL;}

	//read
	while(m_file.read("STATE") != NULL)
	{
		buf = m_file.read(); i = atoi(buf); //state number
		if(m_file.read("~s") != NULL) {buf = m_file.read(); Item->s[i-1] = searchForStates(buf);}
		else { Item->s[i-1] = saveState(NULL); if(Item->s[i-1] == NULL) return NULL; }
	}

	if(m_file.read("~t") != NULL) {buf = m_file.read(); Item->tr = searchForTransp(buf);}
	else{ Item->tr = saveTransp(NULL); if(Item->tr == NULL) return NULL; }

	m_file.read("ENDHMM");

	add(Item);

	return Item;
}

float **CHTKAcousticModel::read2Matrix(unsigned int _x, unsigned int _y, unsigned int _log)
{
	char *buf;
	float **f = NULL;
	unsigned int i,j;
	float l;

	f = new float*[_x];
	for(i=0; i<_x; i++)
	{
		f[i] = new float[_y];
		for(j=0;j<_y;j++)
		{
			buf = m_file.read();
			l = atof(buf);
			if(_log)
			{
				f[i][j] = (l != 0.0) ? (float)log(l) : LOG_ZERO;
				if(f[i][j] != LOG_ZERO && f[i][j] != 0.0) f[i][j] *= (-1.0);
			}
			else {f[i][j] = l;}
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
	for(i=0; i<_x; i++)
	{
		buf = m_file.read();
		f[i] = atof(buf);
	}

	return f;
}

void CHTKAcousticModel::invert(float *_f, unsigned int _size)
{
	unsigned int i;
	for(i=0; i<_size; i++) _f[i] = 1.0/_f[i];
}


EAR_AM_Info *CHTKAcousticModel::getAcousticModel()
{
	//declare
	EAR_AM_Info *out = NULL;
	HTK_State	*s   = NULL;
	HTK_Pdf		*p	 = NULL;
	unsigned int i,j;
	unsigned int pdfsOnState = 0;

	//get number of maximum pdfs on one state
	HTK_State *temp = m_HTK_info.s;
	while(temp != NULL)
	{
		if(pdfsOnState < temp->ipdfs){pdfsOnState = temp->ipdfs;} 
		temp = temp->next;
	}

	//allocate
	out = new EAR_AM_Info;

	out->iVectorSize = m_HTK_info.iVectorSize;
	out->iNumberOfPdfs = m_HTK_info.iPdfs;
	out->iNumberOfStates = m_HTK_info.iStates;
	out->iPdfsOnState = pdfsOnState;
	out->States = new unsigned int*[out->iNumberOfStates];
	for(i=0; i<out->iNumberOfStates; i++)
		out->States[i] = new unsigned int[out->iPdfsOnState];
	out->Pdfs = new EAR_AM_Pdf[out->iNumberOfPdfs];

	//null state array
	for(i=0;i<out->iNumberOfStates;i++)
		for(j=0;j<out->iPdfsOnState;j++)
			out->States[i][j] = NONE;

	//fill pdf arrray
	p = m_HTK_info.pdf;
	while(p!=NULL)
	{
		out->Pdfs[p->id - 1].fVar = cloneVector(p->var, out->iVectorSize);
		out->Pdfs[p->id - 1].fMean = cloneVector(p->mean, out->iVectorSize);
		out->Pdfs[p->id - 1].fgconst = p->gconst;
		out->Pdfs[p->id - 1].fWeight = p->weight;

		p = p->next;
	}

	//go through HTK_info and fill new array
	s = m_HTK_info.s;
	while(s!=NULL)
	{
		for(i=0;i<s->ipdfs;i++)
		{
			p = s->pdfs[i];
			if(p == NULL) out->States[s->id - 1][i] = NONE;
			else out->States[s->id - 1][i] = p->id;
		}	

		s = s->next;
	}

	return out;
}
