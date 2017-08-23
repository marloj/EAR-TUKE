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

#ifndef __EAR_HTK_ACOUSTICMODEL_H_
#define __EAR_HTK_ACOUSTICMODEL_H_

#include <stdio.h>
#include <map>
#include <string.h>

#include "../Data/Data.h"
#include "../Data/FileIO.h"

using namespace std;
namespace Ear
{
	typedef struct HTK_Trans_
	{
		char *name;
		unsigned int id;
		float **p;
		int size;
		HTK_Trans_ *next;
	}HTK_Trans;

	typedef struct HTK_Pdf_
	{
		char *name;
		unsigned int id;
		float *var;
		float *mean;
		float weight;
		float gconst;
		HTK_Pdf_ *next;
	}HTK_Pdf;

	typedef struct HTK_State_
	{
		char *name;
		unsigned int id;
		unsigned int ipdfs;
		HTK_Pdf **pdfs;
		HTK_State_ *next;
	}HTK_State;

	typedef struct HTK_Data_
	{
		unsigned int id;
		short is;
		char *name;
		HTK_Trans *tr;	
		HTK_State **s;
		HTK_Data_ *next;
	}HTK_Data;

	typedef struct HTK_Info_
	{
		short iVectorSize;
		char szType[100];
		unsigned int idata;
		unsigned int iStates;
		unsigned int iPdfs;
		unsigned int iTrs;
		HTK_Data *data;
		HTK_State *s;
		HTK_Pdf	*pdf;
		HTK_Trans *trs;
	}HTK_Info;

	class CHTKAcousticModel
	{
	private:
		struct cmp_str
		{
			bool operator()(char const *a, char const *b)
			{
				return strcmp(a, b) < 0;
			}
		};

	private:
		//structure to hold model parameters
		HTK_Info m_HTK_info;

		//for easy searchíng in data
		map<char*, HTK_Trans*, cmp_str> mapTrs;
		map<char*, HTK_State*, cmp_str> mapStates;
		map<char*, HTK_Data*, cmp_str>  mapData;

		//file reader
		FileIO m_file;

	public:
		CHTKAcousticModel();
		~CHTKAcousticModel();
		int loadModelFrom(char *_szFileName);
		HTK_Data *searchForData(char *_name);
		HTK_Info *getHMMInfo();
		EAR_AM_Info *getAcousticModel();

	private:
		HTK_Trans *searchForTransp(char *_name);
		HTK_State *searchForStates(char *_name);
		float **read2Matrix(unsigned int _x, unsigned int _y, unsigned int _log);
		float *readVector(unsigned int _x);
		void invert(float *_f, unsigned int _size);
		int readHMMHeader();
		int readHMMInfo();
		HTK_Trans *saveTransp(char *_name);
		HTK_State *saveState(char *_name);
		HTK_Data  *saveModel(char *_name);
		HTK_Pdf   *savePdf(char *_name);
		int macroTransp();
		int macroState();
		int macroModel();
		void add(HTK_Trans *_item);
		void add(HTK_State *_item);
		void add(HTK_Data *_item);
		void add(HTK_Pdf *_item);

	public:
		//HTK_Info *getHMMInfo();
	};
}

#endif
