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
	/// check if the network is there.
	if(!_pNet) return EAR_FAIL; m_pNet = _pNet;
	/// set the penalty
	m_fPenalty = _fWordInsPenalty;
	/// set the scorer instance
	m_pScorer = _pScorer;

  /// search for maximum number used for state
	/// this part creates virtual end state that all the end states are connected to.
	/// It is easier to look after one end state as to taking care of numerous end states.
	/// The network has the end state marked as END_STATE macro. But for the decoding process we need to have actual end state number
	/// for inserting the token into stacks under this name. So we need to create one for it.
  m_iEndState = 0;
	for(unsigned int i=0; i<m_pNet->iSize;i++) if(m_pNet->pNet[i].iStart > m_iEndState) m_iEndState = m_pNet->pNet[i].iStart;
	m_iEndState++;	///< use the next availabe state number
	m_iStates = m_iEndState + 1;	///< number of states in the network including zero state that is always initial state of the network

	/// create pool of tokens. Take number of states in search network times 10 number of tokens.
	/// This needs to be optimized for larger networks, but for events detection it is not crucial
	/// as the network tends to be really small.
	m_pTokens = new CTokenPool(10 * m_pNet->iSize);

  /// Prepare viterbi decoding stack. This stack will hold current tokens and token in previous time.
	/// This is one array divided in half to represent previous time and current time tokens respectively.
	/// Those halves change place when new input feature vector is consumed.
  m_iSrc = 0; m_iDst = m_iStates;
	/// allocate two stacks
	m_ppStack = new CToken*[2 * m_iStates];
	/// reset them
	memset(m_ppStack, 0, 2 * m_iStates * sizeof(CToken*));

  /// prepare the decoding process
	reset();

	return EAR_SUCCESS;
}

void CSearch::reset()
{
	/// remove all tokens from the stacks
	unsigned int i=0; CToken *token = NULL;
	for(i=0;i<m_iStates;i++)
	{
		token = prev(i); if(token) {m_pTokens->ret(token);}
		token = cur(i);  if(token) {m_pTokens->ret(token);}
	}

	/// reset stacks
	memset(m_ppStack, 0, 2 * m_iStates * sizeof(CToken*));

	/// create/get from the pool new token
	token = m_pTokens->add(NULL);
	/// insert it to the start state of the network
	insert(token, START_STATE);
	/// reset the symbol of the token
	token->iSym = EPS_SYM;
	/// propagate the token through empty input symbols transitions as they are not consuming input feature vectors when crossing them.
	propagateEmpty(token);
}

unsigned int CSearch::process(CDataContainer &_pData, int64_t _iIndex)
{
	CToken *token = NULL;
	unsigned int i;
	int ret = 0;

	/// no vector available return fail
  if(!_pData.size()){ return EAR_FAIL; }

	/// set new feature vector to the scorer
	ret = m_pScorer->set(&_pData); m_iIndex = _iIndex;
	/// the scorer reported wrong feature vector return fail
	if(ret == EAR_FAIL){ return EAR_FAIL; }

	/// switch the stacks
	nextTime();

	/// go through all tokens from previous time, if there is token in the state, propagate it to the next transitions consuming input symbols.
	/// return the old token from the stack to the pool as the new tokens are copies. The old token will be marked for deletion,
	/// thus not removed to the pool if there are references to it from another tokens.
  for(i=0;i<m_iStates;i++) ///< we have stack of the tokens equal to the number of states, so go though all state numbers.
  {
      token = prev(i);
      if(token){ if(token->iPos != END_STATE) propagateFull(token); m_pTokens->ret(token); }
  }

	return EAR_SUCCESS;
}

void CSearch::propagateEmpty(CToken *_token)
{
	/// loading variables
	unsigned int iPos	= _token->iPos; ///< get position of the current token to propagate (this is the position index in the array of transitions)
	unsigned int iStart = m_pNet->pNet[iPos].iStart; ///< get start state number of the token position (because we are working with the state numbers in viterbi stacks)
	unsigned int iSym  = _token->iSym;	/// get symbol in the token
	unsigned int iState = 0;
	CToken *token = NULL;

	/// last tokens tend to have empty symbol, so we take symbol from the previous token
	/// if it exists. The previous token is guaranteed to have non-empty symbol if it exists.
	if(!iSym && _token->pPrev) iSym = _token->pPrev->iSym;

	/// this is empty symbol propagation, so go through all transition with empty input symbol
	/// and propagate it through the network.
	while(m_pNet->pNet[iPos].iStart == iStart)	///< transitions with the same starting state
	{
		/// if there is transition that does not have end state, increase the position in the network and continue
		if(m_pNet->pNet[iPos].iEnd == UNDEF_STATE) {iPos++; continue;}

		/// the transition has empty input symbol, go through that further
		if(m_pNet->pNet[iPos].iIn == EPS_SYM)
		{
			/// create new token that refers to the one with non-empty output symbol
			if(_token->iSym) token = m_pTokens->add(_token);
			else token = m_pTokens->add(_token->pPrev);

			/// initialize the token from the current one
			token->initToken(_token);

			/// copy the time reference from the current token. We are not consuming input feature vector
			/// so the time stays still.
			token->iIndex = _token->iIndex;

      /// add score found on transition to the token auxiliary score.
			/// include also insertion penalty if the transition has non-empty output symbol
			/// the times minut one means that we are adding back the sign that we have taken from the probabilities
			/// on transition when we were building it.
			if(m_pNet->pNet[iPos].iOut && m_pNet->pNet[iPos].iOut != iSym)
			{
			    token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight + m_fPenalty);
			    token->iSym = m_pNet->pNet[iPos].iOut; ///< the output symbol found on the transition
			}
			else { token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight); }

			/// set the position of the token to the end state (the end state are here the new position in the array)
			/// in loading the FST from file we have replaced end state numbers to the indexes of the array where the particular
			/// transition list starts.
			token->iPos = m_pNet->pNet[iPos].iEnd;

			/// do not forget to propagate the token from the end state further if the new position of the token is not end state.
			if(token->iPos != END_STATE) propagateEmpty(token);

			/// get the position of the new token and insert it to stack
			iState = posToState(token->iPos);
			insert(token, iState);
		}
		/// go to the next transition that belongs to this start state or until end of the transition array.
		iPos++; if(iPos >= m_pNet->iSize) break;
	}
}

void CSearch::propagateFull(CToken *_token)
{
	//variables for cyclus
	unsigned int iPos	= _token->iPos;			///< get the position of the token (this is position in the array of transitions)
	unsigned int iStart = m_pNet->pNet[iPos].iStart;	///< get actual state number of the token's position (needed to insert the token into viterbi stack)
	unsigned int iSym  = _token->iSym; ///< get output symbol stored in the token
	unsigned int iState = 0;
	CToken *token = NULL;

	/// if this is new token and the token does not have crossed output symbol on the any transition so far
	/// take the output symbol of the previous one that this token is referring to.
	/// The previous token is guaranteed to have non-empty output symbol stored.
	if(!iSym && _token->pPrev) iSym = _token->pPrev->iSym;

	/// go through all transition that are starting form the same state
	while(m_pNet->pNet[iPos].iStart == iStart)
	{
		/// if the end state of the transition is undefined, skip and continue next.
	  if(m_pNet->pNet[iPos].iEnd == UNDEF_STATE) {iPos++; continue;}

		/// go through transition that have non-empty output symbol
		if(m_pNet->pNet[iPos].iIn != EPS_SYM)
		{
			/// create new token that is referring to the token provided
			/// or the previous one depending on the non-empty output symbol presence in the referred token.
			if(_token->iSym) token = m_pTokens->add(_token);
			else token = m_pTokens->add(_token->pPrev);

			/// initialize token from the previous one, copy the scores
			token->initToken(_token);

			/// copy the current time reference. The number is increased each time new input feature vector is consumed
			token->iIndex = m_iIndex;

      /// compute auxiliary score, the score found on transitions
			/// include also the penalty if there was output symbol on the transition
			if(m_pNet->pNet[iPos].iOut && m_pNet->pNet[iPos].iOut != iSym)
			{
			    token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight + m_fPenalty);
			    token->iSym = m_pNet->pNet[iPos].iOut;	///< there was non-empty output symbol on this transition
			}
			else { token->addAuxScore((-1)*m_pNet->pNet[iPos].fWeight); }

			/// compute main score from PDFs. The PDFs are referred by the input symbols on the transitions.
			token->addMainScore(m_pScorer->getScore(m_pNet->pNet[iPos].iIn));

			/// get new position of the token. The end state is index in the transition array.
			token->iPos = m_pNet->pNet[iPos].iEnd;

			/// do not forget to propagate the new token through any transition with empty input symbol
			/// leaving from the new state
			if(token->iPos != END_STATE) propagateEmpty(token);

			/// get the number of state from new position or number of end state if the new position is END_STATE
			/// and insert new token into viterbi stack
			iState = posToState(token->iPos);
			insert(token, iState);
		}
		/// go to the next transition.
		iPos++; if(iPos >= m_pNet->iSize) break;
	}
}

void CSearch::insert(CToken *_token, unsigned int _iState)
{
	/// if this is not plausible token, return it to the pool
	if(_iState >= m_iStates){m_pTokens->ret(_token); return;}

	/// correct the index in viterbi stack (convert from state number to the index in the stack's array)
	_iState += m_iDst;
	/// if the state already has a token, but the token has higher score
	/// leave the one already in state intact, remove the new one instead.
	if(m_ppStack[_iState] != NULL && m_ppStack[_iState]->getScore() > _token->getScore())
	{
		m_pTokens->ret(_token); return;
  }
	/// return the old token from the state to the pool and add new one
	m_pTokens->ret(m_ppStack[_iState]);
	m_ppStack[_iState] = _token;
}

CToken *CSearch::prev(unsigned int _iState)
{
	if(_iState >= m_iStates){return NULL;}

	/// correct index depending on which half holds now the previous time tokens
	_iState += m_iSrc;
	return m_ppStack[_iState];
}

CToken *CSearch::cur(unsigned int _iState)
{
	if(_iState >= m_iStates){return NULL;}

	/// correct the index depending on which half now contains the current time tokens.
	_iState += m_iDst;
	return m_ppStack[_iState];
}

void CSearch::nextTime()
{
	/// switch the position indexes of the two stacks
	unsigned int i = m_iDst;
	m_iDst = m_iSrc;
	m_iSrc = i;

	memset(m_ppStack + m_iDst, 0, m_iStates * sizeof(CToken*));
}

unsigned int CSearch::posToState(unsigned int _i)
{
	/// if end state return computed end state number
	if(_i == END_STATE) return m_iEndState;

	return m_pNet->pNet[_i].iStart;
}

CToken *CSearch::getEndStateToken()
{
    return cur(m_iEndState);
}

void CSearch::getResults(CResults &_results)
{
	/// clear the result structure
  _results.clear();

  /// get token from end state
  CToken *pToken = getEndStateToken();

  /// if there is no token in the end state or there is no previous token
	/// return, we do not have any results yet.
  if(pToken == NULL || pToken->pPrev == NULL) return;

  CResult newResult;
	//int64_t iEndIndex = pToken->iIndex;

  //go through all tokens
  while(pToken->pPrev)
	{
		/// start time of the event in number of frames received.
		newResult.iRevIndex = pToken->pPrev->iIndex;
		/// duration of the event (subtracting current and previous tokens indexes)
		newResult.iDur      = pToken->iIndex - pToken->pPrev->iIndex;
		/// number of the acoustic event
		newResult.iId       = pToken->pPrev->iSym;
		/// score of the acoustic event as difference between current and previous token.
		newResult.fScore    = pToken->getScore() - pToken->pPrev->getScore();

		//copy into list of events
		_results.push_front(newResult);

    pToken = pToken->pPrev;
  }
}
