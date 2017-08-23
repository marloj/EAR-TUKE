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

#ifndef __EAR_FSTASSEMBLY_H_
#define __EAR_FSTASSEMBLY_H_

#include "HTKAcousticModel.h"
#include "Dictionary.h"
#include "../Data/Data.h"

#include <map>

using namespace std;

namespace Ear
{
	class StateManager
	{
	public:
		StateManager();
		~StateManager();

	public:
		unsigned int &operator[](unsigned int _i);
		unsigned int ready(unsigned int _start, unsigned int _last, unsigned int _number);
		unsigned int getNewNumber();

	private:
		void size(unsigned int _size);

	private:
		unsigned int *StateArray;
		unsigned int m_size;
		unsigned int m_count;
	};

	class CFSTAssembly
	{
	public:
		CFSTAssembly();
		~CFSTAssembly();

	public:
		int assembly(CHTKAcousticModel *_model, CDictionary *_dict);
		int writeBin(const char *_szOut, const char *_szOutIndex);
		int writeFST(const char *_szFstName, const char *_iSymName, const char *_oSymName);

	private:
		int writeFSTFile(const char *_szFileName);
		int writeOSymFile(const char *_szFileName);
		int writeISymFile(const char *_szFileName);
		void createFST(StateManager &_states, HTK_Data *_hmm, DictItem *_word);
		void connect(unsigned int _s, unsigned int _e, unsigned int _i, unsigned int _o, float _w);

	private:
		multimap<unsigned int, EAR_FST_Trn*> m_fst;
		CHTKAcousticModel *m_model;
		CDictionary *m_dict;
	};

	
}

#endif