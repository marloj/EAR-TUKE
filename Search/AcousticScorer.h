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
 *	This file contains acoustic scorer implementation for scoring PDFs (probabilistic density functions against input)
 * feature vectors.
 */

#ifndef __EAR_ACOUSTICSCORER_H_
#define __EAR_ACOUSTICSCORER_H_

#include "../Data/Data.h"

namespace Ear
{
	/**
	*	This is implementation of scoring mechanism using PDFs and input feature vectors
	* Also includes skipping first N components of the PDFs from scoring if needed.
	* Some results are showing that skipping the basic coefficients from scoring
	* is increasing the accuracy of the detection and classification.
	*/
	class CAcousticScorer
	{
	public:
		CAcousticScorer();
		~CAcousticScorer();

	public:
		/// Setting acoustic model that will be used for scoring purposes.
		/// @param [in] _am acoustic model information in native format
		/// @param [in] _iStrip_offset strip the first offset coefficients
		void setAcousticModel(EAR_AM_Info *_am, unsigned int _iStrip_offset);
		/// Getting the score for particular model. This function provides the scoring computation
		/// @param [in] _Index the index of the state to score
		/// @return total score computed using current vector and PDFs functions belogning to specified state
		float getScore(unsigned int _Index);
		/// Set the current feature vector that will be used for computing scores against PDFs of the states
		/// This function also checks the size of the input feature vector and the acoustic model for compatility in size
		/// @param [in] _vector Container containing the current input feature vector
		/// @return success state of setting new vector for scoring
		int set(CDataContainer *_vector);

	private:
		EAR_AM_Info *am;	///< remembering the acoustic model pointer
		float *scores;	///< scores already computed for particular input feature vector (caching purposes)
		CDataContainer *vector; ///< feature vector the will be used for scoring (current set)
		unsigned int m_iStrip_offset; ///< set offset for scoring.
	};
}

#endif
