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
 *  This file holds the transform functions like Fourier transform and Discrete Cosine transform
 */

#ifndef __EAR_TRANSFORM_H_
#define __EAR_TRANSFORM_H_

#include "../Data/Data.h"

namespace Ear
{
  /**
  * Fast Fourier Implementation. Implementation is using real coefficients as complex
  * and last stage to form the complex output then compute the modul of them.
  */
	class CFourier : public ADataProcessor
	{
	public:
		CFourier();
		virtual ~CFourier();

	private:
		unsigned int m_iSize;  ///< size of the input frame
    unsigned int m_iPower; ///< size of the nearest power of two of the size
    unsigned int m_iFFT;   ///< size of the output fft

	public:
    /// Get new data from this processor.
    /// @param [in, out] _pData Container to be filled with new data
		void getData(CDataContainer &_pData);
	};

  /**
  * Discrete cosine transform for getting output cepstral coefficients.
  * The class is holding internaly the tranform matrix that is precomputed.
  * The output is then computed by simple vector matrix multiplication.
  */
	class CDct : public ADataProcessor
	{
	public:
    /// Initialize the cosine transform.
    /// @param [in] _iSize number of output coefficients
		CDct(unsigned int _iSize);
		virtual ~CDct();

	private:
		unsigned int m_iOutputSize; ///< number of output coefficients
    unsigned int m_iInputSize; ///< input vector size
		float **m_pfCos; ///< table holding the transform values
		CDataContainer m_Tmp; ///< temporary container for input vectors as the vector matrix multiplication can not be done in-place
	public:
    /// Get new data from this processor.
    /// @param [in, out] _pData Container to be filled with new dat
		void getData(CDataContainer &_pData);

	private:
    /// Initialization function of the transform (transform matrix)
		void initDct(unsigned int _iSize);
	};
}

#endif
