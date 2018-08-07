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

	while(item != NULL)
	{
		delete[] item->word;
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
	unsigned int counter = 1; //0 is reserved for empty word
	map<char*, unsigned int, cmp_str> mapWord2Id;
	map<char*, unsigned int, cmp_str>::iterator it;

	if(m_file.open(_szFileName, m_file.READ, " 	") != EAR_SUCCESS) return EAR_FAIL;

	buf = m_file.read();

	while(buf != NULL)
	{
		Item = new DictItem;
		Item->next = NULL;

		Item->word = cloneString(buf);

		//add index
		it = mapWord2Id.begin();
		it = mapWord2Id.find(Item->word);
		if(it != mapWord2Id.end()) Item->id = it->second;
		else {Item->id = counter; mapWord2Id[Item->word] = counter; counter++;}

		buf = m_file.read();
		Item->prob = atof(buf);


		buf = m_file.read();
		Item->models = cloneString(buf);

		buf = m_file.read();

		add(Item);
	}

	m_file.close();

	return EAR_SUCCESS;
}

void CDictionary::add(DictItem *_item)
{
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
