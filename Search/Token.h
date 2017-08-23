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

#ifndef __EAR_TOKEN_H_
#define __EAR_TOKEN_H_

#include <stdlib.h>
#include "../Data/Data.h"

namespace Ear
{
    class CToken
    {
    public:
        CToken(){reset();}
        ~CToken(){}

    public:
        unsigned int iPos, iSym, iUsage; CToken *pPrev;
        int64_t iIndex;
    private:
        float fMainScore, fAuxScore;
        float fNextMainScore;

    public:
        void reset(){iPos = START_STATE; iSym = EPS_SYM; iIndex = 0; fMainScore = 0; fAuxScore = 0; fNextMainScore = 0; pPrev = NULL; iUsage = 0;}

        inline void addAuxScore (float _fScore){ fAuxScore  += _fScore; }
        inline void addMainScore(float _fScore){ fMainScore += fNextMainScore; fNextMainScore = _fScore; }

        inline float getMainScore()     { return fMainScore; }
        inline float getScore()         { return fMainScore + fNextMainScore + fAuxScore; }

        inline void initToken( CToken *_token )
        {
            fMainScore = _token->fMainScore + _token->fNextMainScore;
            fAuxScore  = _token->fAuxScore;

            fNextMainScore = 0;
        }
    };

	class CTokenPool
	{

	private:
		class CHolder
		{
		public:
			CHolder(unsigned int _iMax);
			~CHolder();

		public:
			unsigned int iMax;
			CToken *pAllocs;
			CHolder *pLink;
		};

	private:
		CToken *m_pEls;
		CHolder *m_pHld;
		unsigned int m_iNum;

	public:
		CTokenPool(unsigned int _iNum);
		~CTokenPool();
		CToken *add(CToken *_p);
		void ret(CToken *_p);

	private:
		void chain(CToken *_p, unsigned int _iMax);
		void allocate(unsigned int _iMax);
	};
}

#endif
