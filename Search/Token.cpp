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
#include "Token.h"

#include <stdio.h>

using namespace Ear;

CTokenPool::CHolder::CHolder(unsigned int _iMax)
{
    iMax = _iMax;
    pAllocs = new CToken[_iMax];
    pLink = NULL;
}

CTokenPool::CHolder::~CHolder()
{
    delete[] pAllocs;
}

CTokenPool::CTokenPool(unsigned int _iNum)
{
	m_iNum = _iNum;
	m_pEls = NULL;
	m_pHld = new CHolder(m_iNum);
	chain(m_pHld->pAllocs, m_pHld->iMax);
}

CTokenPool::~CTokenPool()
{
	CHolder *tmp = m_pHld;
	while(tmp)
	{
		m_pHld = m_pHld->pLink;
		delete tmp;
		tmp = m_pHld;
	}
}

void CTokenPool::chain(CToken *_p, unsigned int _iMax)
{
	unsigned int i;
	for(i=0;i<_iMax;i++)
	{
		_p[i].pPrev = m_pEls;
		m_pEls = _p + i;
	}

	m_iNum += _iMax;
}

void CTokenPool::allocate(unsigned int _iMax)
{
	CHolder *tmp = new CHolder(_iMax);
	tmp->pLink = m_pHld;
	m_pHld = tmp;
	chain(tmp->pAllocs, tmp->iMax);
}

CToken *CTokenPool::add(CToken *_p)
{
	if(_p) _p->iUsage++;
	if(!m_pEls) allocate(m_pHld->iMax);

	CToken *tmp = m_pEls;
	m_pEls = tmp->pPrev;

	tmp->reset(); tmp->iUsage++; tmp->pPrev = _p;
	return tmp;
}

void CTokenPool::ret(CToken *_p)
{
	CToken *tmp = NULL;
	if(_p) _p->iUsage--;

	while(_p && _p->iUsage == 0)
	{
		tmp = _p->pPrev; _p->pPrev = m_pEls;
		m_pEls = _p; _p = tmp;
		if(_p) _p->iUsage--;
	}
}
