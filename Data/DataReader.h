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

#ifndef __EAR_DATAREADER_H_
#define __EAR_DATAREADER_H_

#include "Data.h"
#include <map>

namespace Ear
{
	class CDataHolder
	{
	public:
		CDataHolder();
		~CDataHolder();

   	public:
		unsigned int load(const char *_szFileName, const char *_szIndexName);
		EAR_AM_Info *getAcousticData();
		EAR_FST_Net *getFSTData();
		EAR_Dict *getDict();

	private:
		EAR_AM_Info am;
		EAR_FST_Net fst;
		EAR_Dict mapWords;
		std::map<unsigned int, unsigned int> mapStates;
	};
}

#endif
