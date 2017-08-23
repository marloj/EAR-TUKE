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

#include "Transform.h"
#include <math.h>

using namespace Ear;

CFourier::CFourier()
{
	m_iSize = 0;
	m_iPower = 1;
}

CFourier::~CFourier()
{
}

void CFourier::getData(CDataContainer &_pData)
{
	unsigned int k,n1,n2,i,j;
	double bf_im, bf_re, e, a;
	double bf_im1, bf_re1;
	double co, si, cop, sip;

	_pData.clear(); actualize(_pData);
	if(!_pData.size()) return;

	//compute FFT width if needed
	i = 1; j = 0;
	if(m_iSize < _pData.size()) {while(i < _pData.size()){i <<= 1; j++;} m_iSize = i; m_iPower = j;}

	//reserve space
	_pData.reserve(m_iSize);
	_pData.size() = m_iSize;

    n2 = m_iSize / 2; j = 0;
  	for(i = 0; i < m_iSize-1; i+=2)
	{
		if(j > i)
		{
			bf_re = _pData[j];      bf_im = _pData[j+1];
			_pData[j] = _pData[i];   _pData[j+1] = _pData[i+1];
			_pData[i] = bf_re;      _pData[i+1] = bf_im;
		}

		n1 = n2;
		while(j >= n1){j -= n1; n1 /= 2;}
		j += n1;
	}

	n1 = 0; n2 = 2;
	for (i=0; i < m_iPower-1; i++)	/*do N/2 FFT*/
	{
		n1 = n2; n2 = n2 + n2; e = EAR_2PI/n1;

		a = sin(0.5 * e); cop = -2.0 * a * a; sip = sin(e);
        co = 1.0; si = 0.0;

		for (j=0; j < n1; j+=2)
		{
			for (k=j; k < m_iSize; k+=n2)
			{
				bf_re = co * _pData[k+n1] - si * _pData[k+n1+1];		//real part
				bf_im = si * _pData[k+n1] + co * _pData[k+n1+1];		//imaginary part

				_pData[k+n1]	= _pData[k] - bf_re;
				_pData[k+n1+1]	= _pData[k+1] - bf_im;
				_pData[k]		= _pData[k] + bf_re;
				_pData[k+1]		= _pData[k+1] + bf_im;
			}

		 a = co;
         co = co * cop - si * sip + co;
         si = si * cop + a * sip + si;
		}
	}

	/*complete the last stage of fft*/
	n1 = n2; n2 = n1 / 2; e = EAR_2PI/n1; a = e;
	a = sin(0.5 * e); cop = -2.0 * a * a; sip = sin(e);

	co = 1.0 + cop; si = sip;

	for (j=0; j+2 < n2; j+=2)
	{
		bf_re = (_pData[j+2] + _pData[n1-2-j])/2.0; bf_im = (_pData[j+2+1] - _pData[n1-1-j])/2.0;
		bf_re1 = (_pData[j+2+1] + _pData[n1-1-j])/2.0; bf_im1 = (_pData[n1-2-j] - _pData[j+2])/2.0;

	  _pData[j+2]	 = bf_re + co * bf_re1 - si * bf_im1;
	  _pData[j+2+1]  = bf_im + co * bf_im1 + si * bf_re1;
	  _pData[n1-2-j] = bf_re - co * bf_re1 + si * bf_im1;
      _pData[n1-1-j]   = -bf_im + co* bf_im1 + si * bf_re1;

	   a = co;
       co = co * cop - si * sip + co;
       si = si * cop + a * sip + si;
	}

   _pData[0] += _pData[1];
   _pData[1] = 0.0;

   for(i=0, j=0; i<_pData.size(); i+=2, j++) _pData[j] = sqrt((_pData[i] * _pData[i]) + (_pData[i+1] * _pData[i+1]));
   _pData.size() = m_iSize/2; _pData.freq() /= 2;
}

CDct::CDct(unsigned int _iSize) : ADataProcessor()
{
	m_iOutputSize	= _iSize;
	m_iInputSize = 0;
	m_pfCos = NULL;
}


CDct::~CDct()
{
	unsigned int i;

	if(m_pfCos)
	{
		for(i=0;i<m_iOutputSize;i++)
			delete[] m_pfCos[i];

		delete[] m_pfCos;
	}
}

void CDct::getData(CDataContainer &_pData)
{
	unsigned int i,j; float norm; //float x;

	m_Tmp.clear(); actualize(m_Tmp);
	if(!m_Tmp.size()){_pData.size() =  0; return;}

	_pData.reserve(m_iOutputSize); _pData.clear(); _pData.size() = m_iOutputSize;
	norm = sqrt(2.0 / m_Tmp.size());

	if(m_iInputSize != m_Tmp.size()) {initDct(m_Tmp.size());}

	/*
	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0; x = (float)(i+1) * EAR_PI / m_Tmp.size();
		for(j=0; j<m_Tmp.size(); j++)
			_pData[i] += m_Tmp[j] * cos(x*(1+j-0.5));

		_pData[i] *= norm;
	}*/


	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0;
		for(j=0; j<m_Tmp.size(); j++)
			_pData[i] += m_Tmp[j] * m_pfCos[i][j];

		_pData[i] *= norm;
	}

/*
	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0;
		//_pData[i] += 0.5 * m_Tmp[j] * m_pfCos[i][j];

		for(j=0; j<m_iInputSize; j++)
			_pData[i] += m_Tmp[j] * m_pfCos[i][j];

		//_pData[i] /= (float)m_iInputSize;
		_pData[i] *= sqrt(2.0 / m_iInputSize);

	}*/

}

void CDct::initDct(unsigned int _iSize)
{
	unsigned int i,j; float x;

	if(m_pfCos)
	{
		for(i=0;i<m_iOutputSize;i++)
			delete[] m_pfCos[i];

		delete m_pfCos;
	}

	m_iInputSize = _iSize;
	m_pfCos = new float*[m_iOutputSize];

	for(i=0; i<m_iOutputSize; i++)
	{
		x = (float)(i+1) * EAR_PI / _iSize;
		m_pfCos[i] = new float[_iSize];
		for(j=0; j<_iSize; j++)
			m_pfCos[i][j] = cos(x*(1+j-0.5));
	}
}

