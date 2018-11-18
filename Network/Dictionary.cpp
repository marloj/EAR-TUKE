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

#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "Dictionary.h"
#include "../Data/Utils.h"

using namespace std;
using namespace Ear;

CDictionary::CDictionary()
{
	m_Dic = NULL;
}

CDictionary::~CDictionary()
{
	DictItem *item = m_Dic;
	DictItem *tmp  = NULL;

  /// deleting linked list
	while(item != NULL)
	{
		delete[] item->word;  ///< this is array of chars, deleting
		delete[] item->models;
		tmp = item;
		item = item->next;
		delete tmp;
	}
}

int CDictionary::loadFromFile(char *_szFileName)
{
	char *buf = NULL;
	DictItem *Item = NULL;
	unsigned int counter = 1; ///< 0 is reserved for empty words (initializing id count)
	map<char*, unsigned int, cmp_str> mapWord2Id;  ///< mapping words to indexes
	map<char*, unsigned int, cmp_str>::iterator it; ///< iterator of the hash map

  /// opening the file using FILEIO class and tabulator as parsing token
	if(m_file.open(_szFileName, m_file.READ, " 	") != EAR_SUCCESS) return EAR_FAIL;

  /// read first token
	buf = m_file.read();

  /// read until end
	while(buf != NULL)
	{
    /// new dict item
		Item = new DictItem;
		Item->next = NULL;  /// no next item

    /// copy string into word member variable
		Item->word = cloneString(buf);

		/// try to find the event if exists
		it = mapWord2Id.begin();
		it = mapWord2Id.find(Item->word);

    /// if exists take the id and add to the dictionary item
		if(it != mapWord2Id.end()) Item->id = it->second;
    /// if it does not create new id and add to mapping
		else {Item->id = counter; mapWord2Id[Item->word] = counter; counter++;}

    /// read next token (should be probability)
		buf = m_file.read();
		Item->prob = atof(buf);

    /// read next token (should be model name)
		buf = m_file.read();
		Item->models = cloneString(buf);

    /// read next token (should be next dictionary item)
		buf = m_file.read();

    /// add new item to the dictionary
		add(Item);
	}

	m_file.close();

	return EAR_SUCCESS;
}

void CDictionary::add(DictItem *_item)
{
  /// simple implementation of adding new item to the beginning of the linked list
	if(m_Dic == NULL){m_Dic = _item;}
	else
	{
		_item->next = m_Dic;
		m_Dic = _item;
	}
}

DictItem *CDictionary::getDict()
{
	return m_Dic;
}
