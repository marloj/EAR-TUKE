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
 *	Searching/Decoding process for the acoustic event detection.
 */

#ifndef __EAR_SEARCH_H_
#define __EAR_SEARCH_H_

#include "../Data/Data.h"
#include "Token.h"
#include "AcousticScorer.h"

namespace Ear {

    /**
     *	Search/decoding process implementation. Uses the acoustic model, acoustic scorer and finite state network.
     * Makes usage of the tokens for searching and retrieving the results.
     */
    class CSearch {
    public:
        CSearch();
        ~CSearch();

    private:
        CAcousticScorer *m_pScorer; ///< scorer instance to use
        EAR_FST_Net *m_pNet; ///< FST network to use
        /// End state number of the whole network. As the search network can possess more than one end state and in each of them the results of detection can be found
        /// this class created virtual end state, with this number to connect all end states into one by empty symbols on the transitions. This way the network will have
        /// only one state to look for which makes algorithm easier. We also need to create end state representation in viterbi stack, so we need to have number for it.
        /// The classic end state is marked by macro END_STATE.
        unsigned int m_iEndState;
        /// penalty payed when crossing output symbol in the search network. The higher value the more acoustic events detections on output, the lower the value the less
        /// detections or merged into one. The right value needs to be found on development set, or otherwise experimentally set.
        float m_fPenalty;

        CTokenPool *m_pTokens; ///< token pool
        CToken **m_ppStack; ///< stacks of the tokens, half referring to tokens in previous time and the other half to current time.
        unsigned int m_iSrc; ///< beginning of the tokens in previous time in the stack
        unsigned int m_iDst; ///< beginning of the tokens in current time in stack
        unsigned int m_iStates; ///< maximum number of states in the search network
        int64_t m_iIndex; ///< current time index passed to the <i>process</i> function.

    public:
        /// Consume input feature vector, propagate token through the search network. First propagate the token through the non-empty transitions. After that propagate new tokens through
        /// empty transitions.
        /// @param [in] _pData Container containing new feature vector
        /// @param [in] _iIndex time reference to include into tokens (NOTE: this is no longer used, but the time reference is rather computed reversely from last token)
        /// @return success status of the process (when the container is empty or does not match with the acoustic model EAR_FAIL is returned)
        unsigned int process(CDataContainer &_pData, int64_t _iIndex);
        /// Initialize the decoding process. Search for the end state. Creates instance of the pool.
        /// @param [in] _pNet search network
        /// @param [in] _pScorer scorer instance
        /// @param [in] _fWordInsPenalty insertion penalty payed when crossing transitions with non-empty output symbol
        /// @return success of the initialization process
        unsigned int initialize(EAR_FST_Net *_pNet, CAcousticScorer *_pScorer, float _fWordInsPenalty);
        /// Reset the decoding process and prepares new one. All tokens in the stacks are removed (returned to the pool)
        /// the stack are cleared. To the current time stack new token is placed referring to the initial state of the network.
        /// next the propagation of the token through transitions with empty input symbol to another states is done.
        void reset();
        /// Getting the virtual end state token from the current stack
        /// @return pointer to the token.
        CToken *getEndStateToken();
        /// Set penalty that is payed when crossing non-empty output symbol on the search network.
        /// @param [in] _fPen new penalty to set
        void changePenalty(float _fPen);
        /// Get the acoustic events list detected so far.
        /// @param [out] _results reference to the list that will be filled with the acoustic events detected.
        void getResults(CResults &_results);

    private:
        /// Propagate token through all transitions that have empty input symbol. The empty input symbol means that no input feature vector is consumed
        /// @param [in] _token token to be propagated through the empty transitions
        void propagateEmpty(CToken *_token);
        /// Propagate token through all transition that non-empty input symbol. This function uses current set feature vector to score against states of acoustic model
        /// represented by input symbol.
        /// param [in] _token token to propagate through the non-empty transitions.
        void propagateFull(CToken *_token);
        /// Inserts token to the state. Meaning that it inserts token into stack of current time while performing the viterbi conditions.
        /// The token is inserted into state only if its score is larger than the one that is already there. The token with the lowest score
        /// is returned to the pool. If there is no token in the state, the new one is simply inserted there.
        /// @param [in] _token token to insert
        /// @param [in] _iState state of the token to insert to.
        void insert(CToken *_token, unsigned int _iState);
        /// Get token from previos time stack
        /// @param [in] _iState state from which we want to retrieve the token
        /// @return pointer to token
        CToken *prev(unsigned int _iState);
        /// Get token from current time stack
        /// @param [in] _iState state from which we want to retrieve the token
        /// @return pointer to the token.
        CToken *cur(unsigned int _iState);
        /// Converting position of the token to state number. The position of the token is referring to the position in the transitions array
        /// and not directly to state number. The state number can be obtained by looking to the index of the array and getting the start state number.
        /// @param [in] _i position to convert.
        /// @return state number (if the end state is marked as end state, the new end state number is returned)
        unsigned int posToState(unsigned int _i);
        /// switches the stacks. Modifies the indexes <i>m_iSrc</i> and <i>m_iDst</i> and clears the new current time stack from all tokens.
        /// The pointers of the tokens are removed not the actual tokens.
        void nextTime();
    };
} //end of Ear namespace

#endif //END OF __SEARCH_H_
