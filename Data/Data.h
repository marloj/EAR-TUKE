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

#ifndef __EAR_DATA_H_
#define __EAR_DATA_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <list>

#define EAR_PI		3.14159265358979
#define EAR_2PI		6.28318530717959

#define END_STATE UINT_MAX
#define UNDEF_STATE	UINT_MAX - 1
#define START_STATE	0
#define EPS_SYM	0

#define	LOG_ZERO	-1000000
#define NONE UINT_MAX

#define EAR_SUCCESS	1
#define EAR_FAIL	0

namespace Ear
{

    typedef struct
    {
        float           *fVar;
        float           *fMean;
        float           fgconst;
        float           fWeight;
    } EAR_AM_Pdf;

	typedef struct
	{
		unsigned short  iVectorSize;
        unsigned int    iNumberOfPdfs;
        unsigned int    iNumberOfStates;
        unsigned int    iPdfsOnState;
        unsigned int    **States;
        //unsigned int    **Active;
       EAR_AM_Pdf      *Pdfs;
    }EAR_AM_Info;

    typedef struct
    {
    	unsigned int    iStart;
        unsigned int    iEnd;
        unsigned int    iIn;
        unsigned int    iOut;
        float		fWeight;
    }EAR_FST_Trn;

    typedef struct
    {
        EAR_FST_Trn     *pNet;
        unsigned int    iSize;
    }EAR_FST_Net;

    typedef struct
    {
        char            **ppszWords;
        unsigned int    iSize;
    }EAR_Dict;

  	class CDataContainer
	{
	public:
		CDataContainer(){iSize = 0; iCap = 0; pfData = NULL;}
		virtual ~CDataContainer(){ if(pfData) delete[] pfData; }

	private:
		float *pfData;			//actual data vector
		unsigned int iSize;		//Size of inner data vector
		unsigned int iCap;		//capacity of allocated data vector
		unsigned int iFreq;		//source coding frequency

	public:
        float *data(){ return pfData; }
		float &operator[](const unsigned int _i){ return pfData[_i]; }
		inline float &get(const unsigned int _i){ return pfData[_i]; }
		void clear() {
			if(pfData) memset(pfData, 0, iCap * sizeof(float)); iSize = 0;
		}
		void reserve(unsigned int _iSize){
			if(_iSize > iCap) allocate(_iSize); 
			else {
				if(_iSize > iSize) memset(pfData + iSize, 0, (_iSize - iSize) * sizeof(float));
			}
		}
		unsigned int& size(){return iSize;}
		unsigned int& freq(){return iFreq;}

		void copy(float *_p, unsigned int _iSize){
			reserve(_iSize); 
			memcpy(pfData, _p, _iSize * sizeof(float)); 
			iSize = _iSize;
		}

		void add(float *_p, unsigned int _iSize){
			reserve(iSize + _iSize); 
			memcpy(pfData + iSize, _p, _iSize * sizeof(float)); 
			iSize += _iSize;
		}

		unsigned int copy(CDataContainer *_p){ 
			return copy(_p, 0, _p->iSize, 0); 
		}
		unsigned int copy(CDataContainer *_p, unsigned int _iS, unsigned int _iL){ 
			return copy(_p, _iS, _iL, 0); 
		}
		unsigned int add(CDataContainer *_p){ 
			return copy(_p, 0, _p->iSize, 1); 
		}
		unsigned int add(CDataContainer *_p, unsigned int _iS, unsigned int _iL){ 
			return copy(_p, _iS, _iL, 1); 
		}


	private:
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

	//basic type for frontend processor
	class ADataProcessor
	{
	public:
		ADataProcessor(){m_pPrev = NULL;}
		virtual ~ADataProcessor(){}

	private:
		ADataProcessor *m_pPrev;

	public:
		virtual void getData(CDataContainer &_pData) = 0;
		virtual void setSource(ADataProcessor *_pPrev){m_pPrev = _pPrev;}
		virtual ADataProcessor* getSource(){return m_pPrev;}

	protected:
		void actualize(CDataContainer &_pData){if(m_pPrev) m_pPrev->getData(_pData); else _pData.reserve(0);}
	};

	//frontend processor with side processing ... double frontend processor
	class AAuxDataProcessor : public ADataProcessor	{
	public:
		AAuxDataProcessor(): ADataProcessor(){m_pAuxPrev = NULL;}
		virtual ~AAuxDataProcessor(){}

	private:
		AAuxDataProcessor *m_pAuxPrev;

	public:
		virtual void setAuxSource(AAuxDataProcessor *_pAuxPrev){m_pAuxPrev = _pAuxPrev;}
		AAuxDataProcessor* getAuxSource(){return m_pAuxPrev;}

	public:
		virtual void getAuxData(CDataContainer& _pData){getData(_pData);} //if not implemented this function has the same function as getData

	protected:
		void actualizeAux(CDataContainer &_pData){if(m_pAuxPrev) m_pAuxPrev->getAuxData(_pData); else _pData.clear();}
	};

	class CConcat : public AAuxDataProcessor
	{
	public:
		CConcat(){ m_tmp.clear(); }
		virtual ~CConcat(){}

	private:
		CDataContainer m_tmp;

	public:
		void getData(CDataContainer &_pData){
			m_tmp.clear();

			actualize(_pData); if(!_pData.size()){return;}
			actualizeAux(m_tmp);  if(!m_tmp.size()){_pData.clear(); return;}

			_pData.add(&m_tmp);
		}
	};

	class CResult
    {
	public:
		int64_t         iRevIndex; //time from end of hypothesis to beginning of event
		int64_t         iDur;
		unsigned int    iId;
		float           fScore;
	};

	typedef std::list<CResult> CResults;
}

#endif