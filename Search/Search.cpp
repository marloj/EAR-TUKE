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
#include <float.h>

#include "Search.h"

using namespace Ear;

CSearch::CSearch()
{
	m_pScorer = NULL;
	m_pNet = NULL;
	m_pTokens = NULL;
	m_ppStack = NULL;
	m_iDst = 0; m_iSrc = 0; m_iEndState = 0;
}

CSearch::~CSearch()
{
	if(m_pTokens) delete m_pTokens;
	delete[] m_ppStack;
}

void CSearch::changePenalty(float _fPenalty)
{
    m_fPenalty = _fPenalty;
}

unsigned int CSearch::initialize(EAR_FST_Net *_pNet, CAcousticScorer *_pScorer ,float _fWordInsPenalty)
{
	if(!_pNet) return EAR_FAIL; m_pNet = _pNet;
	m_fPenalty = _fWordInsPenalty;
	m_pScorer = _pScorer;

    //find end state number
    m_iEndState = 0;
	for(unsigned int i=0; i<m_pNet->iSize;i++) if(m_pNet->pNet[i].iStart > m_iEndState) m_iEndState = m_pNet->pNet[i].iStart;
	m_iEndState++; m_iStates = m_iEndState + 1;

	//create pool of tokens for decoding
	m_pTokens = new CTokenPool(10 * m_pNet->iSize);

    //prepare viterbi stack
    m_iSrc = 0; m_iDst = m_iStates;
	m_ppStack = new CToken*[2 * m_iStates];
	memset(m_ppStack, 0, 2 * m_iStates * sizeof(CToken*));

    //prepare rest of the decoding process
	reset();

	return EAR_SUCCESS;
}

void CSearch::reset()
{
	unsigned int i=0; CToken *token = NULL;
	for(i=0;i<m_iStates;i++)
	{
		token = prev(i); if(token) {m_pTokens->ret(token);}
		token = cur(i);  if(token) {m_pTokens->ret(token);}
	}

	memset(m_ppStack, 0, 2 * m_iStates * sizeof(CToken*));
	token = m_pTokens->add(NULL); insert(token, START_STATE); token->iSym = EPS_SYM;
	propagateEmpty(token);
}

unsigned int CSearch::process(CDataContainer &_pData, int64_t _iIndex)
{
	CToken *token = NULL; unsigned int i;
	int ret = 0;

    if(!_pData.size()){ return EAR_FAIL; }

	ret = m_pScorer->set(&_pData); m_iIndex = _iIndex;
	if(ret == EAR_FAIL){ return EAR_FAIL; }

	nextTime();

    for(i=0;i<m_iStates;i++)
    {
        token = prev(i);
        if(token){ if(token->iPos != END_STATE) propagateFull(token); m_pTokens->ret(token); }
    }

	return EAR_SUCCESS;
}

void CSearch::propagateEmpty(CToken *_token)
{
	//variables for cyclus
	unsigned int iPos	= _token->iPos;
	unsigned int iStart = m_pNet->pNet[iPos].iStart;
	unsigned int iSym  = _token->iSym;
	unsigned int iState = 0;
	CToken *token = NULL;

	//repair if isym is EPS_SYM
	if(!iSym && _token->pPrev) iSym = _token->pPrev->iSym;

	//cycle through all transition without input symbol
	while(m_pNet->pNet[iPos].iStart == iStart)
	{
	    if(m_pNet->pNet[iPos].iEnd == UNDEF_STATE) {iPos++; continue;}

		if(m_pNet->pNet[iPos].iIn == EPS_SYM)
		{
			//create new token
			if(_token->iSym) token = m_pTokens->add(_token);
			else token = m_pTokens->add(_token->pPrev);

			//initialize token from previous one
			token->initToken(_token);

			//copy time reference
			token->iIndex = _token->iIndex;

            //add scores to token
			if(m_pNet->pNet[iPos].iOut && m_pNet->pNet[iPos].iOut != iSym)
			{
			    token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight + m_fPenalty);
			    token->iSym = m_pNet->pNet[iPos].iOut;
			}
			else { token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight); }

			//compute state number from position and update next position of the token
			token->iPos = m_pNet->pNet[iPos].iEnd;

			//propagate token through next empty transitions
			if(token->iPos != END_STATE) propagateEmpty(token);

			//insert token into stack
			iState = posToState(token->iPos);
			insert(token, iState);
		}
		//next interation or end of transducer field
		iPos++; if(iPos >= m_pNet->iSize) break;
	}
}

void CSearch::propagateFull(CToken *_token)
{
	//variables for cyclus
	unsigned int iPos	= _token->iPos;
	unsigned int iStart = m_pNet->pNet[iPos].iStart;
	unsigned int iSym  = _token->iSym;
	unsigned int iState = 0;
	CToken *token = NULL;

	//repair if isym is EPS_SYM
	if(!iSym && _token->pPrev) iSym = _token->pPrev->iSym;

	//cycle through all transition withou input symbol
	while(m_pNet->pNet[iPos].iStart == iStart)
	{
	    if(m_pNet->pNet[iPos].iEnd == UNDEF_STATE) {iPos++; continue;}

		if(m_pNet->pNet[iPos].iIn != EPS_SYM)
		{
			//create new token
			if(_token->iSym) token = m_pTokens->add(_token);
			else token = m_pTokens->add(_token->pPrev);

			//initialize token from previous one
			token->initToken(_token);

			//copy time reference
			token->iIndex = m_iIndex;

            //add scores to token
			if(m_pNet->pNet[iPos].iOut && m_pNet->pNet[iPos].iOut != iSym)
			{
			    token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight + m_fPenalty);
			    token->iSym = m_pNet->pNet[iPos].iOut;
			}
			else { token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight); }

			token->addMainScore(m_pScorer->getScore(m_pNet->pNet[iPos].iIn));

			//compute state number from position and update next position of the token
			token->iPos = m_pNet->pNet[iPos].iEnd;

			//propagate token through next empty transitions
			if(token->iPos != END_STATE) propagateEmpty(token);

			//insert token into stack
			iState = posToState(token->iPos);
			insert(token, iState);
		}
		//next interation or end of transducer field
		iPos++; if(iPos >= m_pNet->iSize) break;
	}
}

void CSearch::insert(CToken *_token, unsigned int _iState)
{
	if(_iState >= m_iStates){m_pTokens->ret(_token); return;}

	_iState += m_iDst;
	if(m_ppStack[_iState] != NULL && m_ppStack[_iState]->getScore() > _token->getScore())
	{
	    m_pTokens->ret(_token); return;
    }

	m_pTokens->ret(m_ppStack[_iState]);
	m_ppStack[_iState] = _token;
}

CToken *CSearch::prev(unsigned int _iState)
{
	if(_iState >= m_iStates){return NULL;}

	_iState += m_iSrc;
	return m_ppStack[_iState];
}

CToken *CSearch::cur(unsigned int _iState)
{
	if(_iState >= m_iStates){return NULL;}

	_iState += m_iDst;
	return m_ppStack[_iState];
}

void CSearch::nextTime()
{
	unsigned int i = m_iDst;
	m_iDst = m_iSrc;
	m_iSrc = i;

	memset(m_ppStack + m_iDst, 0, m_iStates * sizeof(CToken*));
}

unsigned int CSearch::posToState(unsigned int _i)
{
	if(_i == END_STATE) return m_iEndState;

	return m_pNet->pNet[_i].iStart;
}

CToken *CSearch::getEndStateToken()
{
    return cur(m_iEndState);
}

void CSearch::getResults(CResults &_results)
{
	//first of all clear the old results
    _results.clear();

    //get valid token from end state (the one with non empty symbol)
    CToken *pToken = getEndStateToken();

    //check if decision condition is met
    if(pToken == NULL || pToken->pPrev == NULL) return;

    CResult newResult; 
	//int64_t iEndIndex = pToken->iIndex;

    //go through all tokens
    while(pToken->pPrev)
	{
          
		newResult.iRevIndex = pToken->pPrev->iIndex;
		newResult.iDur      = pToken->iIndex - pToken->pPrev->iIndex;
		newResult.iId       = pToken->pPrev->iSym;
		newResult.fScore    = pToken->getScore() - pToken->pPrev->getScore();

		//copy into list of events
		_results.push_front(newResult);

        pToken = pToken->pPrev;
    }
}
