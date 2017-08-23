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

#ifndef __EAR_FILTER_H_
#define __EAR_FILTER_H_

#include "../Data/Data.h"

namespace Ear
{
	class CLifter : public ADataProcessor
	{
	public:
		CLifter(float _fFactor);
		virtual ~CLifter();

	private:
		float m_fFactor;
		float *m_pfLift;
		unsigned int m_iSize;

	private:
		void initLifter(unsigned int _iSize);

	public:
		void getData(CDataContainer &_pData);
	};

	class CMelBank : public ADataProcessor
	{
	public:
		CMelBank(unsigned int _iMin, unsigned int _iMax, unsigned int _iCount, bool _bLogs);
		virtual ~CMelBank();

	private:
		class CMelFilter
		{
		public:
			CMelFilter(unsigned int _iNum, unsigned int _iMin, unsigned int _iMax)
			{m_iSize = 0; iI = NULL; fW = NULL; m_iMin = _iMin; m_iMax = _iMax; m_iNum = _iNum;}
			virtual ~CMelFilter(){
				if(fW) delete[] fW; if(iI) delete[] iI;
			}

		private:
			unsigned int m_iSize, m_iMin, m_iMax, m_iNum;
			short *iI;
			float *fW;

		public:
			unsigned int size(){return m_iSize;} unsigned int chans(){return m_iNum;}
			short operator()(unsigned int _iI){return iI[_iI];}
			float operator[](unsigned int _iI){return fW[_iI];}
			void init(unsigned int _iSize, float _fs);

		private:
			float linToMel(float freq);
		};

	private:
		CMelFilter *m_fltr;
		CDataContainer m_tmp;
		unsigned int m_iFreq;
		bool m_bLogs;

	public:
		void getData(CDataContainer &_pData);
	};

	class CPreem : public ADataProcessor
	{
	public:
		CPreem(float _fFactor);
		virtual ~CPreem();

	private:
		float m_fFactor;// m_fPrev;

	public:
		void getData(CDataContainer &_pData);
	};

	class CWindow : public ADataProcessor
	{
	public:
		CWindow(float _fFactor);
		virtual ~CWindow();

	private:
		float m_fFactor;
		float *m_pfHam;
		unsigned int m_iSize;

	public:
		void getData(CDataContainer &_pData);
	};

	class CCMN : public ADataProcessor
	{
	public:
		CCMN(unsigned int _iWin);
		virtual ~CCMN();

	private:
		CDataContainer **m_pBuffer;
		CDataContainer *m_pMean;
		//CDataContainer *m_pVar;
		unsigned int m_iWin, m_iFrames;
		unsigned int m_iRead, m_iWrite;
		bool m_bInit, m_bDel;

	public:
		void getData(CDataContainer &_pData);
	};
}

#endif
