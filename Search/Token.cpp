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
  /// go through the inner array of the tokens and prepend them into token's pool linked list of free tokens.
	for(i=0;i<_iMax;i++)
	{
		_p[i].pPrev = m_pEls;
		m_pEls = _p + i;
	}

	m_iNum += _iMax;
}

void CTokenPool::allocate(unsigned int _iMax)
{
  /// allocate new holder and thus new tokens.
	CHolder *tmp = new CHolder(_iMax);
	tmp->pLink = m_pHld;
	m_pHld = tmp;
  /// stack them into linked list of the token pool
	chain(tmp->pAllocs, tmp->iMax);
}

CToken *CTokenPool::add(CToken *_p)
{
  /// take the previous token and increase its usage
	if(_p) _p->iUsage++;
  /// if there is not any token available in pool create new holder and allocated next ones
	if(!m_pEls) allocate(m_pHld->iMax);

  /// get the top token from linked list
	CToken *tmp = m_pEls;
	m_pEls = tmp->pPrev;

  /// initialize it, chain with the previous provided and return
  /// increase also the usage of the new token as the token is needed.
  /// token that usage drops to zero will be removed
	tmp->reset(); tmp->iUsage++; tmp->pPrev = _p;
	return tmp;
}

void CTokenPool::ret(CToken *_p)
{
  ///temporaty variable to go through all token that this token is referring to
	CToken *tmp = NULL;
  /// decrease usage of this token as want to delete it
	if(_p) _p->iUsage--;

  /// if the usage of this token is zero after decresing means that,
  /// no other token is referring to this one...
  /// we will go through the references of the tokens and remove one by one
	while(_p && _p->iUsage == 0)
	{
    /// return the tokens into linked list
		tmp = _p->pPrev; _p->pPrev = m_pEls;
		m_pEls = _p; _p = tmp;
    /// go to next and decrease its usage.
		if(_p) _p->iUsage--;
	}
}
