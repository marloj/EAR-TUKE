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

#include "Filter.h"
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

using namespace Ear;

CLifter::CLifter(float _fFactor) : ADataProcessor()
{
	m_fFactor = _fFactor;
	m_pfLift = NULL;
	m_iSize = 0;
}

CLifter::~CLifter()
{
	if(m_pfLift) delete[] m_pfLift;
}

void CLifter::getData(CDataContainer &_pData)
{
	//float x, y;
	unsigned int i;

	actualize(_pData);
	if(!_pData.size()) return;

	if(m_iSize != _pData.size()) {initLifter(_pData.size());}

	//x = EAR_PI/m_fFactor; y = m_fFactor / 2.0;
	//for(i=0;i<_pData.size();i++) _pData[i] *= 1.0 + y * sin((i+1) * x);

	for(i=0;i<_pData.size();i++) _pData[i] *= m_pfLift[i];
}

void CLifter::initLifter(unsigned int _iSize)
{
	float x, y;
	unsigned int i;

	if(m_pfLift) delete[] m_pfLift;

	m_pfLift = new float[_iSize]; m_iSize = _iSize;

	x = EAR_PI/m_fFactor; y = m_fFactor / 2.0;
	for(i=0;i<_iSize;i++) m_pfLift[i] = 1.0 + y * sin((i+1) * x);
}

CMelBank::CMelBank(unsigned int _iMin, unsigned int _iMax, unsigned int _iCount, bool _bLogs) : ADataProcessor()
{
	m_iFreq = 0; m_tmp.size() = 0;
	m_fltr = new CMelFilter(_iCount, _iMin, _iMax);
	m_bLogs = _bLogs;
}

CMelBank::~CMelBank()
{
	if(m_fltr) delete m_fltr;
}

void CMelBank::getData(CDataContainer &_pData)
{
	unsigned int i; CMelFilter& f = *m_fltr;
	//static int b;
	//b++;

	//actualize data
	m_tmp.clear(); m_tmp.size() = 0; actualize(m_tmp);
	if(!m_tmp.size()) {_pData.clear(); return;}

	//initialize mel filters
	if(f.size() != m_tmp.size() || m_iFreq != m_tmp.freq()) {
		m_iFreq = m_tmp.freq(); 
		f.init(m_tmp.size(), 1/((float)m_tmp.size() * ((int)(1.0E7/m_iFreq))/1.0E7));
	}

	//reserve space
	_pData.reserve(f.chans()); _pData.clear();

	//printf("%d\n", b);

	//apply filter
	for(i=0; i<m_tmp.size(); i++)
	{
		if(f(i) != -1)
		{
			if(f(i) < f.chans()) {_pData[f(i)]	   += m_tmp[i] * f[i];}
			if(f(i) > 0)		 {_pData[f(i) - 1] += m_tmp[i] * (1 - f[i]);}
		}
	}

	_pData.size() = f.chans();

	//compute logs
    if(m_bLogs)
    {
        for(i=0;i<_pData.size();i++)
        {
            if(_pData[i] < 1.0) _pData[i] = 1.0;
            _pData[i] = log(_pData[i]);
        }
    }
}

void CMelBank::CMelFilter::init(unsigned int _iSize, float _fs)
{
	//declarations
	unsigned int i, iEdge;
	float mBegin, mEnd, mSize, x;
	float *mEdges = new float[m_iNum + 2];
	memset(mEdges, 0.0, (m_iNum + 2) * sizeof(float));

	//memory allocation
	if(iI) delete[] iI; if(fW) delete[] fW; m_iSize = _iSize;
	iI = new short[m_iSize]; fW = new float[m_iSize];

	//compute begining and end in mel freq
	mBegin = linToMel(m_iMin); 
	if(m_iMax != UINT_MAX) mEnd = linToMel(m_iMax); 
	else mEnd = linToMel(m_iSize * _fs);

	//compute freqs
	mSize = (mEnd - mBegin)/(float)(m_iNum+1);
	for(i=0;i<m_iNum+2;i++) mEdges[i] = (float)i * mSize + mBegin;

	//compute channels for each coef. and weight
	for(i=0;i<m_iSize;i++)
	{
		iEdge = 0; x = linToMel(i*_fs);
		while(x > mEdges[iEdge]){iEdge++;}
		if(iEdge == 0) {iI[i] = - 1; fW[i] = 0.0;}
		else
		{
			fW[i] = (x - mEdges[iEdge - 1]) / (mEdges[iEdge] - mEdges[iEdge - 1]);
			iI[i] = iEdge - 1;
		}
	}

	//dealocate
	delete[] mEdges;
}

float CMelBank::CMelFilter::linToMel(float freq)
{
	//return (2595.0 * log10(1.0 + freq / 700.0));
	return(1127 * log(1 + freq/700));
}

CPreem::CPreem(float _fFactor) : ADataProcessor()
{
	m_fFactor = _fFactor;
	//m_fPrev = FLT_MAX;
}

CPreem::~CPreem()
{
	//nothing to do here
}

void CPreem::getData(CDataContainer &_pData)
{
	//float tmp;

	//call previous processing
	actualize(_pData);

	if(!_pData.size()) return;

	//if(m_fPrev == FLT_MAX) m_fPrev = _pData[0];

	//tmp = _pData[_pData.size()-1];
	for(unsigned int i=_pData.size()-1; i>0; i--) _pData[i] -= _pData[i-1] * m_fFactor;
	_pData[0] *= 1.0 - m_fFactor;

	//m_fPrev = tmp;
}

CWindow::CWindow(float _fFactor) : ADataProcessor()
{
	m_fFactor = _fFactor;
	m_pfHam = NULL;
	m_iSize = 0;
}

CWindow::~CWindow()
{
	if(m_pfHam) delete[] m_pfHam;
}

void CWindow::getData(CDataContainer &_pData)
{
	unsigned int i;

	actualize(_pData);
	if(!_pData.size()) return;

	if(!m_pfHam || _pData.size() != m_iSize)
	{
		if(m_pfHam) delete[] m_pfHam;
		m_iSize = _pData.size();
		m_pfHam = new float[m_iSize];
		for(i=0; i<m_iSize; i++) m_pfHam[i] = (1-m_fFactor) - (m_fFactor * cos((2 * EAR_PI * i)/(m_iSize - 1)));
	}

	for(i=0; i<m_iSize; i++) _pData[i] *= m_pfHam[i];
}

CCMN::CCMN(unsigned int _iWin) : ADataProcessor()
{
    m_iWin = _iWin; m_iFrames = 0; m_iRead = 0; m_iWrite = 0; m_bInit = false;
	m_pBuffer = new CDataContainer*[_iWin];
	for(unsigned int i = 0; i<_iWin; i++) {m_pBuffer[i] = new CDataContainer(); m_pBuffer[i]->clear();}
	m_pMean = new CDataContainer();
	//m_pVar = new CDataContainer();
	m_pMean->clear();
	//m_pVar->clear();

	m_bDel = false;
}

CCMN::~CCMN()
{
	for(unsigned int i=0;i<m_iWin;i++) delete[] m_pBuffer[i];
	delete[] m_pBuffer;
	delete m_pMean;
	//delete m_pVar;
}

void CCMN::getData(CDataContainer &_pData)
{
    unsigned int j;

	if(!m_bInit)
	{
	    //initializing for computation
        for(m_iWrite=0; m_iWrite<m_iWin; m_iWrite++)
        {
            //read data to buffer to write position
            m_pBuffer[m_iWrite]->clear();
            actualize(*(m_pBuffer[m_iWrite]));

            //rezervacia miesta pre mean
            if(!m_pBuffer[m_iWrite]->size()) {_pData.size() = 0; return;}
            m_pMean->reserve(m_pBuffer[m_iWrite]->size());

            //add to mean
            for(j=0; j<m_pBuffer[m_iWrite]->size(); j++)
            {
                (*(m_pMean))[j] += (*(m_pBuffer[m_iWrite]))[j];
            }
        }

        m_bInit = true;
	}

	if(m_bInit && m_bDel)
	{
	    //go to next write position
        m_iWrite++;
        if(m_iWrite >= m_iWin) m_iWrite -= m_iWin;

        //update mean
        for(j=0;j<_pData.size();j++)
        {
            (*(m_pMean))[j] -= (*(m_pBuffer[m_iWrite]))[j];
        }

        //get new data
        actualize(*(m_pBuffer[m_iWrite]));

        //update mean
        for(j=0; j<m_pBuffer[m_iWrite]->size(); j++)
        {
                (*(m_pMean))[j] += (*(m_pBuffer[m_iWrite]))[j];
        }
	}


    if(m_pBuffer[m_iRead]->size() == 0){_pData.size() = 0; return;}

    //copy data from read position
    _pData.copy(m_pBuffer[m_iRead]);

    //use mean
    for(j=0; j<_pData.size(); j++)
    {
        _pData[j] -= (*(m_pMean))[j] / (float)m_iWin;
    }

    //gon to the nex reeading position
    m_iRead++; if(m_iRead >= m_iWin) m_iRead -= m_iWin;

    if(m_iRead >= m_iWin/2){m_bDel = true;}
}
