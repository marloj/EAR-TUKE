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

 /**
 *  This file contains the Token and Token Poll definition. Token is and object traveling
 * the search finite state transducer network. Token pool is preallocated memory, from which
 * tokens are created.
 */

#ifndef __EAR_TOKEN_H_
#define __EAR_TOKEN_H_

#include <stdlib.h>
#include "../Data/Data.h"

namespace Ear
{
    /**
    * Token, an object traveling the finite state transducer search network. The token is holding curcial information
    * about the search process and score. From the token at the end of decoding or in the process of the decoding
    * a final or partial results respectively can be retrieved.
    * The tokens in the network are refering to each other. Each time new input vector is consumed new copies of the current tokens
    * are created. The new token are referring the older ones that have crossed non-empty output symbol on transition of the serch network
    */
    class CToken
    {
    public:
        CToken(){reset();}
        ~CToken(){}

    public:
        unsigned int iPos;    ///< position of the token in the search network (state number)
        unsigned int iSym;  ///< consumed output symbol from the search network transition
        unsigned int iUsage; ///< number of references to this token by another tokens. If this drops to zero, the token is removed from the memory
        CToken *pPrev;  ///< reference pointer to the previous non-empty (containing symbol) token
        int64_t iIndex; ///< time index of the token in number of feature vectors.
    private:
        float fMainScore; ///<
        float fAuxScore;
        float fNextMainScore;

    public:
        /// Reset the state of the token to default values
        void reset(){iPos = START_STATE; iSym = EPS_SYM; iIndex = 0; fMainScore = 0; fAuxScore = 0; fNextMainScore = 0; pPrev = NULL; iUsage = 0;}

        inline void addAuxScore (float _fScore){ fAuxScore  += _fScore; }
        inline void addMainScore(float _fScore){ fMainScore += fNextMainScore; fNextMainScore = _fScore; }

        inline float getMainScore()     { return fMainScore; }
        inline float getScore()         { return fMainScore + fNextMainScore + fAuxScore; }

        /// initialize token with existent token. Copy the values of the token to this instance
        /// @param [in] _token the token from which copy the values
        inline void initToken( CToken *_token )
        {
            fMainScore = _token->fMainScore + _token->fNextMainScore;
            fAuxScore  = _token->fAuxScore;

            fNextMainScore = 0;
        }
    };

  /**
  * Pool of tokens that were preallocated or allocated during search process. This is faster than allocating new tokens
  * from operating system. The tokens are allocated by the inner holder class and chained for use. The reason behind this
  * is simple. The holders are allocating the tokens in one go, while the pool have linked list of them. The deallocation
  * is simple too. Dealocating the holders removes all the tokens, so no token will be forgotten in the memory. This prevents
  * memory leaks.
  */
	class CTokenPool
	{

	private:
    /**
    * Holder of the tokens. Inner class of one time allocated resources. Holds token and it is also part of the
    * linked list of the holders. Each time all tokens are used from the pool new tokens are allocated by creating
    * new holder.
    */
		class CHolder
		{
		public:
      /// Allocate new tokens.
      /// @param [in] _iMax maximum tokens to create
			CHolder(unsigned int _iMax);
			~CHolder();

		public:
			unsigned int iMax; ///< remembering maximum number of tokens
			CToken *pAllocs; ///< allocated tokens array
			CHolder *pLink; ///< link to next token holder
		};

	private:
		CToken *m_pEls; ///< Tokens allocated by the holders and created a linked list of them here (this are free tokens available to the search process)
		CHolder *m_pHld;  ///< Linked list of the holders
		unsigned int m_iNum; ///< NUmber of tokens created by the pool (increases each time new holder is created)

	public:
    /// Creating token pool
    /// @param [in] _iNum number of token to preallocate for one holder. Each time no more tokens are available the next specified number will be allocated.
		CTokenPool(unsigned int _iNum);
		~CTokenPool();
    /// Take one token from the pool chaing with the provided one and return new initialized token.
    /// This also increases the usage of the previous token for counting references. The new token has initial usage equal to one as we need it.
    /// If we want to delete it we will decrease the usage later.
    /// @param [in] _p previous token to link the new one
    /// @return new allocated token take from the pool
		CToken *add(CToken *_p);
    /// Return token back to the pool. This also decreases the usage/reference of the previous token.
    /// @param [in] _p token to return
		void ret(CToken *_p);

	private:
    /// Chaining tokens.
		void chain(CToken *_p, unsigned int _iMax);
		void allocate(unsigned int _iMax);
	};
}

#endif
