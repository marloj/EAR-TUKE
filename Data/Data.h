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
 * This file contains basic common definition of the:
 * 1. Constants used across the whole software
 * 2. Acoustic model structure
 * 3. Finite State Transducer structure (search network)
 * 4. Dictionary structure
 * 5. Data Container definition used for representing waveform and feature vectors.
 * 6. Processor definition used for feature vector extraction process.
 * 7. Result of the detection definition
 */

#ifndef __EAR_DATA_H_
#define __EAR_DATA_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <list>

/// defining PI for easy use in computations later
#define EAR_PI		3.14159265358979
/// defining 2 * PI for easy use in computations later
#define EAR_2PI		6.28318530717959

/// default number for defining end state in finite state transducer
#define END_STATE UINT_MAX
/// default number for defining undefined state in finite state transducer
/// to define transition than has not been connected to end state yet
#define UNDEF_STATE	UINT_MAX - 1
/// define default start state number
#define START_STATE	0
/// define default epsilon symbol number
/// the symbol is default empty symbol used on transitions
#define EPS_SYM	0
/// as the log(0) is -infinity, we define it as very low number
#define	LOG_ZERO	-1000000
/// generaly define none as maximum integer number
#define NONE UINT_MAX

/// success constant definition used as return value from funtions
#define EAR_SUCCESS	1
/// fail constant definition used as return value from functions
#define EAR_FAIL	0

namespace Ear
{
  /// defining the structure of the multidimensional Gauss Probability Density function
  /// for the acoustic model
  typedef struct
  {
      float           *fVar;    ///< variance of the PDF
      float           *fMean;   ///< mean of the PDF
      float           fgconst;  ///< precomputed gcons value of the PDF
      float           fWeight;  ///< weight of the PDF among others PDF for the same state in the model
  } EAR_AM_Pdf;

  /// defining top structure of the acoustic model. We do not need the names of the models here, because the FST
  /// refers to the states on its transitions by number in the <i>States</i> array.
	typedef struct
	{
	   unsigned short  iVectorSize;    ///< feature vector dimensionality, also the PDF dimensionality
     unsigned int    iNumberOfPdfs;  ///< number of PDFs in the model for all states
     unsigned int    iNumberOfStates;///< number of the states in the model
     unsigned int    iPdfsOnState;   ///< number of PDFs on one state
     unsigned int    **States;       ///< array of the PDF indexes for each state in acoustic model
     EAR_AM_Pdf      *Pdfs;          ///< array of all PDFs in the acoustic model
  }EAR_AM_Info;

  /**
  * defining transition structure for the finite state transducer (search network)
  * the transition are store typicaly in an array, thus the indexes for start and end state
  * are indexes in that array
  */
  typedef struct
  {
    unsigned int    iStart; ///< start state number index in the array of transitions
    unsigned int    iEnd;   ///< end state number index in the array of transitions
    unsigned int    iIn;    ///< consuming (in) symbol index on the transition
    unsigned int    iOut;   ///< emiting (out) symbol index on the transition, mapping to names are in EAR::EAR_Dict
    float		fWeight;        ///< weight associated with the transition
  }EAR_FST_Trn;

  /// defining top structure for the finite state transducer (search network)
  typedef struct
  {
    EAR_FST_Trn     *pNet;  ///< array of the transitions
    unsigned int    iSize;  ///< size of the array
  }EAR_FST_Net;

  /// defining dictionary, the mapping from output symbols indexes to the names of acoustic events
  typedef struct
  {
    char            **ppszWords;  ///< array of the events names
    unsigned int    iSize;        ///< size of the dictionary
  }EAR_Dict;

  /**
  * Class for holding the feature vector for the whole time when it passes through preprocessing.
  * Contains convenient functions for appending, prepending, copying data to another container
  * In many way the container resembles <vector> from C++
  */
	class CDataContainer
	{
	public:
    /// constructor, all set to default values
		CDataContainer(){iSize = 0; iCap = 0; pfData = NULL;}
    /// destructor, desctruct inner array that holds the data
		virtual ~CDataContainer(){ if(pfData) delete[] pfData; }

	private:
		float *pfData;			  ///< array holding data, the array is dynamically reallocated if needed
		unsigned int iSize;		///< size of the data in the array
		unsigned int iCap;		///< size of the data that the array can hold
		unsigned int iFreq;		///< sampling frequency of the data in the container

	public:
    /// returns the inner data array pointer
    /// @return pointer to the inner data array
    float *data(){ return pfData; }
    /// operator of square brackets to access the number in the container by index
    /// @return reference to the number on the index position
		float &operator[](const unsigned int _i){ return pfData[_i]; }
    /// get function to get number from container by index
    /// @param [in] _i index in the array
    /// @return reference to the number on the index position
		inline float &get(const unsigned int _i){ return pfData[_i]; }
    /// Clears the content of the container. The function does not deallocate the inner array
    /// but writes zeroes the array memory and sets size of the data content to zero
		void clear() {
			if(pfData) memset(pfData, 0, iCap * sizeof(float)); iSize = 0;
		}
    /// reserves space for the data. The function reallocates the inner array if needed and copy old data
    /// to new array. This does not change the size of the content in the container only its capacity
    /// @param [in] _iSize new size of the container's capacity
		void reserve(unsigned int _iSize){
      /// reallocate if the new size is larger than the available capacity
			if(_iSize > iCap) allocate(_iSize);
			else {
        /// capacity is sufficient, just zero the remaining part
				if(_iSize > iSize) memset(pfData + iSize, 0, (_iSize - iSize) * sizeof(float));
			}
		}
    /// Function to manually manipulate size of the data in the container
    /// @return reference to the size member variable to change
		unsigned int& size(){return iSize;}
    /// Function to manually manipulate sampling frequency of the data in container
    /// @return reference to the frequency member variable to change
		unsigned int& freq(){return iFreq;}

    /// Copies data from input float array to the container. Resulting size of the data will be equal to the size of input array
    /// @param [in] _p pointer to the input array
    /// _iSize [in] size of the input array
		void copy(float *_p, unsigned int _iSize){
			reserve(_iSize);
			memcpy(pfData, _p, _iSize * sizeof(float));
			iSize = _iSize;
		}

    /// Appends data from input array to the data in container. Resulting size of the data container will
    /// be equal to the sum of the new array size and size of the data already present in the container
    /// @param [in] _p array to append
    /// @param [in] _iSize size of the array to append
		void add(float *_p, unsigned int _iSize){
			reserve(iSize + _iSize);
			memcpy(pfData + iSize, _p, _iSize * sizeof(float));
			iSize += _iSize;
		}

    /// Copies data from another container
    /// @param _p [in] data container pointer to copy from
    /// @return size of the data copied (always is equal to the input size)
		unsigned int copy(CDataContainer *_p){
			return copy(_p, 0, _p->iSize, 0);
		}
    /// Copies data from another container from specified start and length
    /// @param _p [in] data container pointer to copy from
    /// @param _iS [in] start index of the data to copy
    /// @param _iL [in] length of the data to copy
    /// @return size of the data copied
		unsigned int copy(CDataContainer *_p, unsigned int _iS, unsigned int _iL){
			return copy(_p, _iS, _iL, 0);
		}
    /// Appends data from another container to the current one
    /// @param _p [in] container to copy data from
    /// @return size of the data copied
		unsigned int add(CDataContainer *_p){
			return copy(_p, 0, _p->iSize, 1);
		}
    /// Appends data from another container with specific start index and length to the current one.
    /// @param _p [in] data container pointer to copy from
    /// @param _iS [in] start index of the data to copy
    /// @param _iL [in] length of the data to copy
    /// @return size of the data copied
		unsigned int add(CDataContainer *_p, unsigned int _iS, unsigned int _iL){
			return copy(_p, _iS, _iL, 1);
		}

	private:
    /// Function for allocating the inner array of the container. The function copies old data if there
    /// were any in the container and also zeroes the remaining allocated.
    /// @param [in] _iSize size of the new inner array (not the data size)
		void allocate(unsigned int _iSize){
			float *p = pfData;
			pfData = new float[_iSize];
			iCap = _iSize;
			memset(pfData, 0.0, iCap * sizeof(float));
			if(p && iSize){
				memcpy(pfData,p,sizeof(float)*iSize);
			}
			if(p) delete[] p;
		}

    /// Copy function implementation used by every add or copy public functions that are taking another container.
    /// The function also changes the size of the data available in container.
    /// @param [in] _p another container to copy or append data from
    /// @param [in] _iStart start position of the data in the container to copy or append
    /// @param [in] _iLength size of the data in the container to copy or append
    /// @param [in] _bAppend whether the new data should be appended or copied and remove old data if any are present.
    /// @return size of the data copied or appended
		unsigned int copy(CDataContainer *_p, unsigned int _iStart, unsigned int _iLength, unsigned int _bAppend){
			if(_p->iSize - _iStart < _iLength) _iLength = _p->iSize - _iStart;

			if(_bAppend) {
				reserve(iSize + _iLength);
			} else {
				reserve(_iLength);
				clear();
			}

    	memcpy(pfData + iSize, _p->pfData + _iStart, _iLength * sizeof(float));
			iSize += _iLength;
			iFreq = _p->iFreq;

			return _iLength;
		}
	};

	/**
  * Basic class for data preprocessor, from which the front-end is made by chaining them.
  * The processors are representing linked list of the processing of the input waveform to the feature vectors.
  */
	class ADataProcessor
	{
	public:
		ADataProcessor(){m_pPrev = NULL;}
		virtual ~ADataProcessor(){}

	private:
		ADataProcessor *m_pPrev; ///< Previous data processor instance

	public:
    /// Function providing processed data. This function may invoke the same function of previous processor to get
    /// new data for processing. Each specific processing implementation derived from this class needs must have this function implemented.
    /// @param [in, out] Container instance that will be filled with new data.
		virtual void getData(CDataContainer &_pData) = 0;
    /// Settings previous processor instance, from which new data can be requested if needed. This function can be implemented
    /// by the derived classes if the specific implementation requires some additional processing or validation when
    /// new soure of data is added to this processor.
    /// @param [in] _pPrev processor instance that serves as the source of new data for the current one
		virtual void setSource(ADataProcessor *_pPrev){m_pPrev = _pPrev;}
    /// Returns previous processor instance that is serving as the source of new data for the current processor
    /// @return pointer to the previous processor.
		virtual ADataProcessor* getSource(){return m_pPrev;}

	protected:
    /// Function that is accessible only by derived classes for requesting new data. If the previous processor was set,
    /// the function calls <i>getData</i> of the processor. Otherwise sets zero length of data for the container.
    /// @param [in, out] _pData Container to be filled with new data. Zero length of the data means that no new data will be available
		void actualize(CDataContainer &_pData){if(m_pPrev) m_pPrev->getData(_pData); else _pData.reserve(0);}
	};

	/**
  * Auxiliary/additional processor. This class is making branching of the processing graph possible.
  * In one case it computes additional data that holds internally until requested by <i>getAuxData</i>.
  * Another function is a processor that takes output from more than two processors.
  */
	class AAuxDataProcessor : public ADataProcessor	{
	public:
		AAuxDataProcessor(): ADataProcessor(){m_pAuxPrev = NULL;}
		virtual ~AAuxDataProcessor(){}

	private:
		AAuxDataProcessor *m_pAuxPrev; ///< pointer to the additional processor instance

	public:
    /// Function for settings additional processor that will serve as additional source of data
    /// @param [in] _pAuxPrev pointer to the additional processor. This function can be implemented by
    /// specific processor implementation when additional validation or processing is needed.
		virtual void setAuxSource(AAuxDataProcessor *_pAuxPrev){m_pAuxPrev = _pAuxPrev;}
    /// Function getting pointer to the previous additional processsor
    /// @return pointer to the additional processor
		AAuxDataProcessor* getAuxSource(){return m_pAuxPrev;}

	public:
    /// Function for getting new data from this additional processor. If not implemented by derived class
    /// the function has the same effect as calling the <i>getData</i> of this processor. Processors implementing
    /// this function serve as dividing points of the preprocessing chain.
    /// @param [in, out] _pData Container to filled with the new data
		virtual void getAuxData(CDataContainer& _pData){getData(_pData);}

	protected:
    /// Function used internaly by derived classes to request data from previous additional processor. If the container
    /// has zero length means that no more data will be available in the future.
    /// @param [in, out] _pData Container to be filled with the new data.
		void actualizeAux(CDataContainer &_pData){if(m_pAuxPrev) m_pAuxPrev->getAuxData(_pData); else _pData.clear();}
	};

  /**
  * Class for processors that are concatenating results of processing from two previous processors.
  * Currently this is used after the computation of the energy, zero and delta coefficients.
  */
	class CConcat : public AAuxDataProcessor
	{
	public:
		CConcat(){ m_tmp.clear(); }
		virtual ~CConcat(){}

	private:
		CDataContainer m_tmp; ///< internal data container to hold data from previous additional processor.

	public:
    /// Function for getting new processed data from both of the processors.
    /// @param [in, out] _pData Container to be filled with new data.
		void getData(CDataContainer &_pData){
			m_tmp.clear();
      /// get data from previous processor
			actualize(_pData); if(!_pData.size()){return;}
      /// get data from previous auxiliary/additional processor
			actualizeAux(m_tmp);  if(!m_tmp.size()){_pData.clear(); return;}
      /// appednd the data
			_pData.add(&m_tmp);
		}
	};

  /**
  * Class holding the result of the processing. Currently the CSearch implementation of the decoding process
  * is using this class for representing results. This class is representing one detection or hypothesis
  */
	class CResult
  {
	public:
	  int64_t         iRevIndex; ///< time from end of hypothesis to the beginning of the event detected
	  int64_t         iDur;      ///< duration of the event detected
    unsigned int    iId;       ///< id of the event detected (output symbol index)
		float           fScore;    ///< score (likelihood) of the detection
	};

  /**
  * typedef for representing the result as array of the CResult classes.
  */
	typedef std::list<CResult> CResults;
}

#endif
