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

#ifndef __DICTIONARYLOADER_H_
#define __DICTIONARYLOADER_H_

#include <string.h>
#include "../Data/FileIO.h"

namespace Ear
{
	typedef struct DictItem_
	{
		char *word;
		unsigned int id;
		float prob;
		char *models;
		DictItem_ *next;
	}DictItem;

	class CDictionary
	{
	public:
		CDictionary();
		~CDictionary();
		int loadFromFile(char *_szFileName);
		DictItem *getDict();

	private:
		void add(DictItem *_item);
		struct cmp_str
		{
		 	bool operator()(char const *a, char const *b) const
		 	{
		 		return strcmp(a, b) < 0;
		 	}
		};

	private:
		FileIO m_file;
		DictItem *m_Dic;
	};
}

#endif
