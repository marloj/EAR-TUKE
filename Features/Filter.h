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
 * This file contains filters implementations:
 * Lifter: filter in the cepstral domain
 * Mel-Bank: Mel filter frequency bank
 * Preemphasis: To emphasize higher frequencies on input
 * Hamming: To filter the signal before Fourier transform
 * CMN: Ceptral mean normalization of the cepstral coefficients
 */

#ifndef __EAR_FILTER_H_
#define __EAR_FILTER_H_

#include "../Data/Data.h"

namespace Ear
{
  /**
  * Filter in the cepstral domain (Lifter).
  * Configuration parameter: _fFactor.
  */
	class CLifter : public ADataProcessor
	{
	public:
    /**
    * Initializes the lifter according setting
    * @param [in] _fFactor factor of the filter
    */
		CLifter(float _fFactor);
		virtual ~CLifter();

	private:
		float m_fFactor;  ///< remembering the filter factor
		float *m_pfLift; ///< initialized lifter coefficients
		unsigned int m_iSize; ///< size of the input feature vector

	private:
    /// initialize the lifter's coefficients
    /// @param [in] _iSize the new size of the input vector
		void initLifter(unsigned int _iSize);

	public:
    /// Get new data from this processor
    /// @param [in, out] _pData Container to be filled with new data
		void getData(CDataContainer &_pData);
	};

  /**
  * Mel frequency bank of filters, from each of the filters one number is computed.
  */
	class CMelBank : public ADataProcessor
	{
	public:
    /**
    * Initialize the filter
    * @param [in] _iMin minimum frequency
    * @param [in] _iMax maximum frequency (UINT_MAX equals to nyqist frequency)
    * @param [in] _iCount number of filters (size of the output vector)
    * @param [in] _bLogs compute log of the output coefficients (false for MELSPEC coeffs. true otherwise)
    */
		CMelBank(unsigned int _iMin, unsigned int _iMax, unsigned int _iCount, bool _bLogs);
		virtual ~CMelBank();

	private:
    /**
    * Inner class for holding the filters. The filters are overlapping triangles, meaning that each frequency magnitude can
    * belong to multiple filters. We are remembering only the increasing slope of the rectangle, while the decreasing
    * side is the same but reversed.
    */
		class CMelFilter
		{
		public:
      /**
      * Initializing the filters.
      * @param [in] _iNum number of filters
      * @param [in] _iMin minimum frequency
      * @param [in] _iMax maximum frequency
      */
			CMelFilter(unsigned int _iNum, unsigned int _iMin, unsigned int _iMax)
			{m_iSize = 0; iI = NULL; fW = NULL; m_iMin = _iMin; m_iMax = _iMax; m_iNum = _iNum;}
			virtual ~CMelFilter(){
				if(fW) delete[] fW; if(iI) delete[] iI;
			}

		private:
			unsigned int m_iSize, m_iMin, m_iMax, m_iNum;
			short *iI; ///< index of the filter that the data belong to
			float *fW; ///< magnitude of the filters (the increasing parts)

		public:
      /// @return size of the filter
			unsigned int size(){return m_iSize;}
      /// @return number of channels (filters)
      unsigned int chans(){return m_iNum;}
      /// operator for getting the number of filter
      /// @return number of coefficient that the filter belongs to
			short operator()(unsigned int _iI){return iI[_iI];}
      /// operator for getting the value of the filter
      /// @return magnitude of the filter
			float operator[](unsigned int _iI){return fW[_iI];}
      /// initialize the filter
      /// @param [in] _iSize size of the filter (number of frequency values from FFT)
      /// @param [in] _fs sampling frequency to compute the FFT resolution
			void init(unsigned int _iSize, float _fs);

		private:
      /// Function to convert the linear frequency scale into mel's one
      /// @param [in] freq frequency
      /// @return mel frequency
			float linToMel(float freq);
		};

	private:
		CMelFilter *m_fltr; ///< pointer to the mel filter inner class instance
		CDataContainer m_tmp; ///< temporary feature vector holder
		unsigned int m_iFreq; ///< remembering the sampling frequency
		bool m_bLogs; ///< flag, whether to compute logs from output coefficient

	public:
    /// Getting new data from this processor
    /// @param [in, out] _pData Container to be filled with new data
		void getData(CDataContainer &_pData);
	};

  /**
  * Preemphasis computation to emphasize higher frequencies.
  */
	class CPreem : public ADataProcessor
	{
	public:
    /**
    * Initializes the filter for preemphasis
    * @param [in] _fFactor the factor, from which to initialize the filter
    */
		CPreem(float _fFactor);
		virtual ~CPreem();

	private:
		float m_fFactor; ///< preemphasis filter factor

	public:
    /// Getting new data from this processor
    /// @param [in, out] _pData Container to be filled with new data
		void getData(CDataContainer &_pData);
	};

  /**
  * Apply window function to the input vector. This often needed as preprocessing
  * for Fourier transform.
  */
	class CWindow : public ADataProcessor
	{
	public:
    /// Initialize the window and set factor
    /// @param [in] _fFactor the factor of the window to set
		CWindow(float _fFactor);
		virtual ~CWindow();

	private:
		float m_fFactor;  ///< remembering factor
		float *m_pfHam; ///< hamming window coefficients
		unsigned int m_iSize; ///< size of the window created

	public:
    /// Getting new data from this processor
    /// @param [in, out] _pData Container to be filled with new data
		void getData(CDataContainer &_pData);
	};

  /**
  * Compute and apply cepstral mean normalization using floating window. Also remember last
  * computed mean of the cepstral coefficients. Contains buffer, that hols all vectors for the window,
  * from which the mean is computed. In this buffer always the older one vector is rewritten with the
  * new one getting new mean computed. The computed mean is then subtracted from the input features.
  * The mean from vectors in the first half of the buffer the CMN computed from the buffer is used.
  * For the next vectors the buffer is updated. At the beginning the buffer is filled with vectors,
  * meaning that it will call <i>getData</i> of previous processor multiple times depending on the
  * window length.
  */
	class CCMN : public ADataProcessor
	{
	public:
    /// Initialize the mean normalization by setting the window length for computation of the mean.
    /// @param [in] _iWin the length of the window
		CCMN(unsigned int _iWin);
		virtual ~CCMN();

	private:
		CDataContainer **m_pBuffer; ///< Container array buffer from which the mean is computed, always the older one is rewritten with new one
		CDataContainer *m_pMean; ///< Computed mean from the buffer
		unsigned int m_iWin; ///< remembering the size of the window
    unsigned int m_iFrames; ///< Not currently used
		unsigned int m_iRead; ///< read cursor of the output feature vector
    unsigned int m_iWrite; ///< write cursor for the input feature vector
		bool m_bInit; ///< whether the buffer is initiated
    bool m_bDel; ///< whether new vector needs to be pushed into buffer.

	public:
    /// Getting new data from this processor
    /// @param [in, out] _pData Container to be filled with new data
		void getData(CDataContainer &_pData);
	};
}

#endif
