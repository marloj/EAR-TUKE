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

	m_pBuffer = new CDataContainer*[m_iBf];
	for(unsigned int i = 0; i<m_iBf; i++) {m_pBuffer[i] = new CDataContainer(); m_pBuffer[i]->clear();}
	iDummy = 0;
}
	
CDelta::~CDelta()
{
	for(unsigned int i=0;i<m_iBf;i++) delete m_pBuffer[i];
	delete[] m_pBuffer;
}

void CDelta::getData(CDataContainer &_pData)
{
	unsigned int i,j,norm;

	rotate(); m_pBuffer[0]->clear(); actualize(*(m_pBuffer[0]));
	if(!m_pBuffer[0]->size() && !iDummy){_pData.size() = 0; return;}

	if(m_pBuffer[0]->size()) {m_iSize = m_pBuffer[0]->size(); m_iSize /= m_iOrd;}

	if(!iDummy && m_pBuffer[0]->size())
	{
		for(i=0;i<m_iBf/2;i++) {rotate(); m_pBuffer[0]->copy(m_pBuffer[1]); iDummy++;}
		for(i=0;i<m_iBf/2;i++) 
		{
			rotate(); m_pBuffer[0]->clear(); actualize(*(m_pBuffer[0]));
			if(!m_pBuffer[0]->size()) {m_pBuffer[0]->copy(m_pBuffer[1]); iDummy--;}
		}	
	}

	if(!m_pBuffer[0]->size())
	{
		m_pBuffer[0]->copy(m_pBuffer[1]);
		iDummy--;
	}

	_pData.copy(m_pBuffer[m_iWin]);

	_pData.reserve((m_iOrd+1)*m_iSize); _pData.size() = (m_iOrd+1)*m_iSize;
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
