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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "PushSource.h"

using namespace Ear;

CPushSource::CPushSource(unsigned int _iLength, unsigned int _iReadLength) : ADataProcessor()
{
    m_iWrite = 0;
    m_iRead = 0;
    m_iSize = _iLength;
    m_pfBuf = new float[_iLength];
    m_iFreq = 0;
    m_iReadLength = _iReadLength;

    m_bEndOfStream = false;
}

CPushSource::~CPushSource()
{
    delete[] m_pfBuf;
}

void CPushSource::openStream()
{
    m_bEndOfStream = false;
}

void CPushSource::closeStream()
{
    m_bEndOfStream = true;
}

void CPushSource::changeFreq(unsigned int _iFreq)
{
   m_iFreq = _iFreq;
}

unsigned int CPushSource::pushData(float *_pfData, unsigned int _iSize)
{
    unsigned int iRead = m_iRead;
    unsigned int iAvail = 0;
    bool bOver = false;

    //compute available space for insertion
    if(iRead > m_iWrite){ iAvail = iRead - m_iWrite - 1; }
    else{ iAvail = m_iSize - m_iWrite + m_iRead - 1; }

    //check overflowing of the buffer
    if(_iSize > iAvail){ bOver = true; _iSize = iAvail;}

    //copy data into buffer
    if(m_iWrite + _iSize <= m_iSize)
    {
        memcpy(m_pfBuf + m_iWrite, _pfData, _iSize * sizeof(float));
    }
    else
    {
        memcpy(m_pfBuf + m_iWrite, _pfData, (m_iSize - m_iWrite) * sizeof(float));
        memcpy(m_pfBuf, _pfData, (_iSize - (m_iSize - m_iWrite)) * sizeof(float));
    }

    //adjust write pointer position
    if(m_iWrite + _iSize >= m_iSize){ m_iWrite =  m_iWrite + _iSize - m_iSize; }
    else{ m_iWrite = m_iWrite + _iSize; }

    //return status of the buffer
    if(bOver) return EAR_FAIL;

    return EAR_SUCCESS;
}

void CPushSource::getData(CDataContainer &_pData)
{
    unsigned int iWrite, iAvail;

    //printf("%d, %d\n", m_iWrite, m_iRead);

    //check if new data available in buffer or end of stream has been reached
    if(m_bEndOfStream){ _pData.clear(); return; }
    //if(m_bEndOfStream && m_iRead == m_iWrite){ _pData.clear(); return; }
    while(m_iRead == m_iWrite){ sleep(1); }

    //compute available data in buffer
    iWrite = m_iWrite;
    if(m_iRead <= iWrite){ iAvail = iWrite - m_iRead; }
    else{ iAvail = m_iSize - m_iRead + iWrite; }

    //adjust available data to max read length
    if(iAvail > m_iReadLength){ iAvail = m_iReadLength; }

    //copy new data into data container
    _pData.reserve(iAvail);
    if(m_iRead + iAvail <= m_iSize)
    {
        _pData.copy(m_pfBuf + m_iRead, iAvail);
    }
    else
    {
        _pData.copy(m_pfBuf + m_iRead, m_iSize - m_iRead);
        _pData.add(m_pfBuf, iAvail - (m_iSize - m_iRead));
    }

    //adjust read pointer position
    if(m_iRead + iAvail >= m_iSize){ m_iRead =  m_iRead + iAvail - m_iSize; }
    else{ m_iRead = m_iRead + iAvail; }

    //set frequency of the input data
    _pData.freq() = m_iFreq;
}
