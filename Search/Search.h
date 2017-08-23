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

#ifndef __EAR_SEARCH_H_
#define __EAR_SEARCH_H_

#include "../Data/Data.h"
#include "Token.h"
#include "AcousticScorer.h"

namespace Ear
{
	class CSearch
	{
	public:
		CSearch();
		~CSearch();

	private:
		//resources
		CAcousticScorer *m_pScorer; EAR_FST_Net *m_pNet;
		unsigned int m_iEndState; float m_fPenalty;

		//Data in processing
		CTokenPool *m_pTokens; CToken **m_ppStack;
		unsigned int m_iSrc, m_iDst, m_iStates;
		int64_t m_iIndex;

	public:
		unsigned int process(CDataContainer &_pData, int64_t _iIndex);
		unsigned int initialize(EAR_FST_Net *_pNet, CAcousticScorer *_pScorer, float _fWordInsPenalty);
		void reset();
		CToken *getEndStateToken();
		void changePenalty(float _fPen);
		void getResults(CResults &_results);

	private:
		void propagateEmpty(CToken *_token);
		void propagateFull(CToken *_token);
		void insert(CToken *_token, unsigned int _iState);
		CToken *prev(unsigned int _iState);
		CToken *cur(unsigned int _iState);
		unsigned int posToState(unsigned int _i);
		void nextTime();
	};
} //end of Ear namespace

#endif //END OF __SEARCH_H_
