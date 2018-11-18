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

#include "Coeffs.h"
#include <math.h>

using namespace Ear;

CDelta::CDelta(unsigned int _iWin, unsigned int _iOrd) : ADataProcessor()
{
	m_iWin = _iWin; m_iBf = 2*(m_iWin) + 1; m_iOrd = _iOrd;

  /// allocate new buffer
	m_pBuffer = new CDataContainer*[m_iBf];
  /// and create instancies
	for(unsigned int i = 0; i<m_iBf; i++) {m_pBuffer[i] = new CDataContainer(); m_pBuffer[i]->clear();}
  /// no dummy vector yet
	iDummy = 0;
}

CDelta::~CDelta()
{
  /// delete everything
	for(unsigned int i=0;i<m_iBf;i++) delete m_pBuffer[i];
	delete[] m_pBuffer;
}

void CDelta::getData(CDataContainer &_pData)
{
	unsigned int i,j,norm;

  /// rotate the buffer, clear the first one. The first one in buffer is the last received. Get new data from previous processor
	rotate(); m_pBuffer[0]->clear(); actualize(*(m_pBuffer[0]));

  /// if we do not have any dummy vectors and the received container is empty return empty container and we are done.
	if(!m_pBuffer[0]->size() && !iDummy){_pData.size() = 0; return;}

  /// we have first data, then we remember the size of the vector.
  /// if we have used this processor multiple times, the order is needed to compute
  /// the length of the basic coefficients or length of the previous order coefficients
	if(m_pBuffer[0]->size()) {m_iSize = m_pBuffer[0]->size(); m_iSize /= m_iOrd;}

  /// if we have data and no dummy vectors, we need to create them.
	if(!iDummy && m_pBuffer[0]->size())
	{
    /// push copied dummy vectors into buffer.
		for(i=0;i<m_iBf/2;i++) {rotate(); m_pBuffer[0]->copy(m_pBuffer[1]); iDummy++;}
    /// push new data from previous processor to fill the remaining part of the buffer
		for(i=0;i<m_iBf/2;i++)
		{
			rotate(); m_pBuffer[0]->clear(); actualize(*(m_pBuffer[0]));
      /// if do not have enough data to fill the buffer we need to replicate the last one
      /// this should be less than number of dummy vectors added at the begining
			if(!m_pBuffer[0]->size()) {m_pBuffer[0]->copy(m_pBuffer[1]); iDummy--;}
		}
	}

  /// if we have not received data and we have the buffer full
  /// replicate the last buffer at most the number of added dummy vector at the beginning
	if(!m_pBuffer[0]->size())
	{
		m_pBuffer[0]->copy(m_pBuffer[1]);
		iDummy--;
	}

  /// copy the middle buffer to the output container
	_pData.copy(m_pBuffer[m_iWin]);

  /// extend its length
	_pData.reserve((m_iOrd+1)*m_iSize); _pData.size() = (m_iOrd+1)*m_iSize;

  /// compute the coefficients
	for(i=(m_iOrd-1)*m_iSize;i<m_iOrd*m_iSize;i++)
	{
		norm = 0; _pData[m_iSize+i] = 0.0;
		for(j=1;j<=m_iWin;j++){_pData[m_iSize + i] += j * ((*(m_pBuffer[m_iWin-j]))[i] - (*(m_pBuffer[m_iWin+j]))[i]); norm += j*j;}
		_pData[m_iSize+i] /= 2*norm;
	}
}

void CDelta::rotate()
{
	CDataContainer *tmp;   unsigned int i;

    tmp = m_pBuffer[2*m_iWin];

    for(i=2*m_iWin; i>0; i--)
			m_pBuffer[i] = m_pBuffer[i-1];

    m_pBuffer[0] = tmp;
}

CEnergy::CEnergy() : AAuxDataProcessor()
{
	m_fEnergy = 0.0;
}

CEnergy::~CEnergy()
{
}

void CEnergy::getData(CDataContainer &_pData)
{
	unsigned int i; m_fEnergy = 0.0;
	actualize(_pData); if(!_pData.size()){return;}

	for(i=0;i<_pData.size();i++) m_fEnergy += _pData[i] * _pData[i];
	m_fEnergy = log(m_fEnergy);
}

void CEnergy::getAuxData(CDataContainer &_pData)
{
	_pData.reserve(1); _pData.clear();
	_pData[0] = m_fEnergy; _pData.size() = 1;
}

CZeroCoef::CZeroCoef() : AAuxDataProcessor()
{
	m_fC0 = 0.0;
}

CZeroCoef::~CZeroCoef()
{
}

void CZeroCoef::getData(CDataContainer &_pData)
{
	unsigned int i; m_fC0 = 0.0;
	actualize(_pData); if(!_pData.size()){return;}

	for(i=0;i<_pData.size();i++) m_fC0 += _pData[i];
	m_fC0 *= sqrt(2.0 / _pData.size());
}

void CZeroCoef::getAuxData(CDataContainer &_pData)
{
	_pData.reserve(1); _pData.clear();
	_pData[0] = m_fC0; _pData.size() = 1;
}
