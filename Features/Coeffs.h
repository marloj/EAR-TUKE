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
 *  This file contains computing coefficients additional to basic
 *  Mel-Frequency Ceptral Coefficients MFCCs
 */

#ifndef __EAR_COEFFS_H_
#define __EAR_COEFFS_H_

#include "../Data/Data.h"

namespace Ear
{
  /**
  * Class for computing delta coefficients. Internally contains circular buffer for frames coefficients
  * and computes delta coefs from this buffer. At the beginning and end of the recording duplicates first and last
  * frame vector respectively to always have the buffer full. At the beginning the buffer replicates first buffer to have
  * the buffer half full and reads the rest frames from input. At the end it replicates the last frame to the other
  * half of the buffer. This also means that this processor is delaying the output depending on the iWin parameter.
  * This also means that one call to the <i>getData</i> function triggers calling the same function of previous processor
  * multiple times.
  * The internal buffer holds number of frames equal to two times the window length plus one for which we are computing
  * the coefficients.
  * For example if the window length is 2 then:
  * v4 basic_coefs
  * v3 basic_coefs
  * v2 basic_coefs, delta_coefs = 1 * (v3 - v1) + 2 * (v4 - v0)
  * v1 basic_coefs
  * v0 basic_coefs
  *
  * the normalizing factor can be computed as norm = 2 * (1*1 + 2*2)
  * the new coefficients are appended to the original, basic coefficients
  */
	class CDelta : public ADataProcessor
	{
	public:
    /**
    * Constructes and initializes the processor.
    * @param [in] _iWin window length of the delta coefficients computation
    * @param [in] _iOrd the order of the delta coefficients (first or second acceleration)
    */
		CDelta(unsigned int _iWin, unsigned int _iOrd);
		virtual ~CDelta();

	private:
		CDataContainer **m_pBuffer; ///< designated buffer of the input frames, it is two times the window length + 1
		unsigned int iDummy;  ///< how many dummy (copied) vectors we have in buffer
    unsigned int m_iWin; ///< remembering the window length
    unsigned int m_iBf; ///< buffer size
    unsigned int m_iSize; ///< size of the vector frames in buffer
    unsigned int m_iOrd; ///< remembering the order of the coefficients computing

	public:
    /**
    * Getting new data from processor. This function can trigger calling <i>getData</i>
    * of previous processor multiple times depending on the window length setting. At the end
    * this function can still provide data without triggering the function for reading data.
    * The function returns empty container if no further data are available
    * @param [in, out] _pData Container to be filled with new data
    */
		void getData(CDataContainer &_pData);

	private:
    /// rotate function for internal buffer
		void rotate();
	};

  /**
  * Computing log energy coefficient for the MFCCs
  */
	class CEnergy : public AAuxDataProcessor
	{
	public:
		CEnergy();
		virtual ~CEnergy();

	public:
    /**
    * Get new data, computes the energy coefficient and remembers it.
    * The unchanged data from previous processor are returned in this function
    * @param [in, out] _pData Container to be filled with new data
    */
		void getData(CDataContainer &_pData);
    /**
    * Get the energy computed from last call of <i>getData</i> function
    * @param [in, out] _pData Container to be filled with the energy coefficient
    */
		void getAuxData(CDataContainer &_pData);

	private:
		float m_fEnergy; ///< for remembering the last computed energy coefficient
	};

  /**
  * Computing zero MFCC coefficient, that is in reality similar to the energy one.
  */
	class CZeroCoef : public AAuxDataProcessor
	{
	public:
		CZeroCoef();
		virtual ~CZeroCoef();

	public:
    /**
    * Get new data, computes the zero MFCC coefficient and remembers it.
    * The unchanged data from previous processor are returned in this function
    * @param [in, out] _pData Container to be filled with new data
    */
		void getData(CDataContainer &_pData);
    /**
    * Get the zero MFCC computed from last call of <i>getData</i> function
    * @param [in, out] _pData Container to be filled with the zero coefficient
    */
		void getAuxData(CDataContainer &_pData);

	private:
		float m_fC0; ///< remembering zero coefficient
	};
}

#endif
