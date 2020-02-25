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
    if (m_pfLift) delete[] m_pfLift;
}

void CLifter::getData(CDataContainer &_pData)
{
    unsigned int i;

    actualize(_pData);
    if (!_pData.size()) return;

    /// initialize lifter coefficients if the vector size was changed and at the beginning
    if (m_iSize != _pData.size()) {
        initLifter(_pData.size());
    }

    //x = EAR_PI/m_fFactor; y = m_fFactor / 2.0;
    //for(i=0;i<_pData.size();i++) _pData[i] *= 1.0 + y * sin((i+1) * x);

    /// apply the filter by simple multplication of the coefficients
    for (i = 0; i < _pData.size(); i++) _pData[i] *= m_pfLift[i];
}

void CLifter::initLifter(unsigned int _iSize)
{
    float x, y;
    unsigned int i;

    /// delete if already exists
    if (m_pfLift) delete[] m_pfLift;

    /// allocate new
    m_pfLift = new float[_iSize];
    m_iSize = _iSize;

    /// compute the coefficients
    x = EAR_PI / m_fFactor;
    y = m_fFactor / 2.0;
    for (i = 0; i < _iSize; i++) m_pfLift[i] = 1.0 + y * sin((i + 1) * x);
}

CMelBank::CMelBank(unsigned int _iMin, unsigned int _iMax, unsigned int _iCount, bool _bLogs) : ADataProcessor()
{
    m_iFreq = 0;
    m_tmp.size() = 0;
    /// create the filter bank
    m_fltr = new CMelFilter(_iCount, _iMin, _iMax);
    m_bLogs = _bLogs;
}

CMelBank::~CMelBank()
{
    if (m_fltr) delete m_fltr;
}

void CMelBank::getData(CDataContainer &_pData)
{
    unsigned int i;
    CMelFilter& f = *m_fltr;
    //static int b;
    //b++;

    /// get new data for processing
    m_tmp.clear();
    m_tmp.size() = 0;
    actualize(m_tmp);
    /// no data, then return empty
    if (!m_tmp.size()) {
        _pData.clear();
        return;
    }

    /// initialize mel filters if the vector size was changed or the sampling frequency
    if (f.size() != m_tmp.size() || m_iFreq != m_tmp.freq()) {
        /// set the frequency
        m_iFreq = m_tmp.freq();
        /// set the size of the input vector and frequency resolution
        f.init(m_tmp.size(), 1 / ((float) m_tmp.size() * ((int) (1.0E7 / m_iFreq)) / 1.0E7));
    }

    /// of the output container
    _pData.reserve(f.chans());
    _pData.clear();

    //printf("%d\n", b);

    /// Apply filter and output
    /// go through all coefficients of the input vector
    for (i = 0; i < m_tmp.size(); i++) {
        /// if the coefficient belongs to some filter
        if (f(i) != -1) {
            /// if this is not last filter, we compute the increasing part of the triangle filter
            if (f(i) < f.chans()) {
                _pData[f(i)] += m_tmp[i] * f[i];
            }
            /// if this is also not first filter, we compute the decreasing part of the triangle and
            /// add this value to the previous coefficient where it belongs to.
            if (f(i) > 0) {
                _pData[f(i) - 1] += m_tmp[i] * (1 - f[i]);
            }
        }
    }

    /// the output is the same as number of filters
    _pData.size() = f.chans();

    /// compute the logs of the output coefficients if needed
    if (m_bLogs) {
        for (i = 0; i < _pData.size(); i++) {
            if (_pData[i] < 1.0) _pData[i] = 1.0;
            _pData[i] = log(_pData[i]);
        }
    }
}

void CMelBank::CMelFilter::init(unsigned int _iSize, float _fs)
{
    //declarations
    unsigned int i, iEdge; ///< working variable
    float mBegin; ///< begining frequency of the filters
    float mEnd; ///< end frequency of the filters
    float mSize; ///< size of one filter
    float x; ///< working variable
    float *mEdges = new float[m_iNum + 2]; ///< all edges frequencies + the beginning and end
    memset(mEdges, 0.0, (m_iNum + 2) * sizeof (float)); ///< set to zero

    /// allocate the same size as the input frequency resolution
    if (iI) delete[] iI;
    if (fW) delete[] fW;
    m_iSize = _iSize;
    iI = new short[m_iSize];
    fW = new float[m_iSize];

    /// compute frequencies in mel domain
    mBegin = linToMel(m_iMin);
    if (m_iMax != UINT_MAX) mEnd = linToMel(m_iMax);
    else mEnd = linToMel(m_iSize * _fs);

    /// compute the edges frequencies of the triangles
    mSize = (mEnd - mBegin) / (float) (m_iNum + 1);
    for (i = 0; i < m_iNum + 2; i++) mEdges[i] = (float) i * mSize + mBegin;

    /// computing the triangles
    for (i = 0; i < m_iSize; i++) {
        iEdge = 0;
        x = linToMel(i * _fs); ///< get the frequency in mel for i-th coefficient
        while (x > mEdges[iEdge]) {
            iEdge++;
        } ///< find where it belongs to
        if (iEdge == 0) {
            iI[i] = -1;
            fW[i] = 0.0;
        } ///< if nowhere mark it as -1
        else {
            /// if it belongs to somewhere, compute the value of the triangle filter for it
            fW[i] = (x - mEdges[iEdge - 1]) / (mEdges[iEdge] - mEdges[iEdge - 1]);
            iI[i] = iEdge - 1; ///< set here the number of the filter of i-the coefficient
        }
    }

    /// dealocate
    delete[] mEdges;
}

float CMelBank::CMelFilter::linToMel(float freq)
{
    //return (2595.0 * log10(1.0 + freq / 700.0));
    return (1127 * log(1 + freq / 700));
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

    /// no input, return empty
    if (!_pData.size()) return;

    //if(m_fPrev == FLT_MAX) m_fPrev = _pData[0];

    //tmp = _pData[_pData.size()-1];
    /// compute preemphasis
    for (unsigned int i = _pData.size() - 1; i > 0; i--) _pData[i] -= _pData[i - 1] * m_fFactor;
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
    if (m_pfHam) delete[] m_pfHam;
}

void CWindow::getData(CDataContainer &_pData)
{
    unsigned int i;

    /// get new data
    actualize(_pData);
    /// no input, return empty
    if (!_pData.size()) return;

    /// create new hamming window if the size of input vector changes
    if (!m_pfHam || _pData.size() != m_iSize) {
        if (m_pfHam) delete[] m_pfHam;
        m_iSize = _pData.size();
        m_pfHam = new float[m_iSize];
        for (i = 0; i < m_iSize; i++) m_pfHam[i] = (1 - m_fFactor) - (m_fFactor * cos((2 * EAR_PI * i) / (m_iSize - 1)));
    }

    /// multiply the window with the input data
    for (i = 0; i < m_iSize; i++) _pData[i] *= m_pfHam[i];
}

CCMN::CCMN(unsigned int _iWin) : ADataProcessor()
{
    m_iWin = _iWin;
    m_iFrames = 0;
    m_iRead = 0;
    m_iWrite = 0;
    m_bInit = false;
    /// allocate buffer
    m_pBuffer = new CDataContainer*[_iWin];
    /// create new containers
    for (unsigned int i = 0; i < _iWin; i++) {
        m_pBuffer[i] = new CDataContainer();
        m_pBuffer[i]->clear();
    }
    /// allocate new mean
    m_pMean = new CDataContainer();
    //m_pVar = new CDataContainer();
    m_pMean->clear();
    //m_pVar->clear();

    m_bDel = false;
}

CCMN::~CCMN()
{
    for (unsigned int i = 0; i < m_iWin; i++) delete[] m_pBuffer[i];
    delete[] m_pBuffer;
    delete m_pMean;
    //delete m_pVar;
}

void CCMN::getData(CDataContainer &_pData)
{
    unsigned int j;

    /// needs to be initialized
    if (!m_bInit) {
        /// initialize whole buffer
        for (m_iWrite = 0; m_iWrite < m_iWin; m_iWrite++) {
            /// get new data to the write position
            m_pBuffer[m_iWrite]->clear();
            actualize(*(m_pBuffer[m_iWrite]));

            /// get the size of the vectors and reserve space for mean
            /// if there is not enough data to process, return empty container
            if (!m_pBuffer[m_iWrite]->size()) {
                _pData.size() = 0;
                return;
            }
            m_pMean->reserve(m_pBuffer[m_iWrite]->size());

            /// compute the mean (actually only add the vectors together now and divide by number of the vectors in buffer later)
            /// Using this method we can subtract the vector that is about to be removed from buffer and add new one, instead
            /// of computing the mean from whole buffer for each input vector
            for (j = 0; j < m_pBuffer[m_iWrite]->size(); j++) {
                (*(m_pMean))[j] += (*(m_pBuffer[m_iWrite]))[j];
            }
        }

        /// initialized
        m_bInit = true;
    }

    /// we need to push into buffer new vector
    /// this happends when the read pointer reaches half of the window from the beginning
    if (m_bInit && m_bDel) {
        /// go to next write position
        m_iWrite++;
        if (m_iWrite >= m_iWin) m_iWrite -= m_iWin; /// the cursors goes in circles

        /// remove the the old vector that is about to be actualized
        for (j = 0; j < _pData.size(); j++) {
            (*(m_pMean))[j] -= (*(m_pBuffer[m_iWrite]))[j];
        }

        /// actualize the vector (get new data there)
        actualize(*(m_pBuffer[m_iWrite]));

        /// add the new vector to the mean
        for (j = 0; j < m_pBuffer[m_iWrite]->size(); j++) {
            (*(m_pMean))[j] += (*(m_pBuffer[m_iWrite]))[j];
        }
    }

    /// no input, return empty container
    if (m_pBuffer[m_iRead]->size() == 0) {
        _pData.size() = 0;
        return;
    }

    /// copy data from the read position
    _pData.copy(m_pBuffer[m_iRead]);

    /// apply the mean (here we divide each coefficient by the number of vectors in buffer)
    for (j = 0; j < _pData.size(); j++) {
        _pData[j] -= (*(m_pMean))[j] / (float) m_iWin;
    }

    /// adjust the read position (goes in circles)
    m_iRead++;
    if (m_iRead >= m_iWin) m_iRead -= m_iWin;

    /// start to load new vectors when the read position is in the middle
    /// FIX: maybe this should be set permanently after that
    if (m_iRead >= m_iWin / 2) {
        m_bDel = true;
    }
}
