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
	unsigned int i,j,k,w,N,T;
	double re,im,wre,wim,a1,a2,b1,b2;

	/// get new data
	_pData.clear(); actualize(_pData);
  /// getting nothing, return empty container
	if(!_pData.size()) return;

	/// compute fft width if the size of the input vector changed
	i = 1; j = 0;
	if(m_iSize < _pData.size()) {
		while(i < _pData.size()){i <<= 1; j++;}
		m_iSize = i;
		m_iPower = j - 1;
		m_iFFT = m_iSize / 2; /// we will get only half of the spectrum on the output side
	}

	/// reserve space in the output container for computation
  /// we used the same container for getting new data, it is ok, the algorithm works in-place
	_pData.reserve(m_iSize);
	_pData.size() = m_iSize;

	/// standard approach to FFT bit-reversing the coefficients first
  /// while threating them asi complex values in order Re(0) Im(1) Re(2) Im(2)
	for(i = 0, j = 0; i < m_iSize - 1; i+=2) {
		if(j > i)	{
			a1 = _pData[j];      		b1 = _pData[j+1];
			_pData[j] = _pData[i];  _pData[j+1] = _pData[i+1];
			_pData[i] = a1;      		_pData[i+1] = b1;
		}

		k = m_iFFT;
		while(j >= k){ j -= k; k /= 2; }  ///< finding the bit-reversed position
		j += k;
	}

	/// FFT butterfly computation of the complex input
	for (N = 2; N <= m_iFFT; N += N) {
		re = cos(EAR_2PI / N);	wre = 1.0;
		im = sin(EAR_2PI / N);	wim = 0.0;
		T = N / 2; /// max twidlle factor

		for (k = 0; k < T; k++) {
			for (j = k; j < m_iFFT; j += N) {
				i = 2 * j; /// actual position in the array (having complex values next real ones)
				w = i + 2 * T; /// actual position in the array
				a1 = wre * _pData[w] - wim * _pData[w+1];
				b1 = wre * _pData[w+1] + wim * _pData[w];
				_pData[w] = _pData[i] - a1;
				_pData[w+1] = _pData[i+1] - b1;
				_pData[i] += a1;
				_pData[i+1] += b1;
			}
      /// do not need to compute the W^k in each iteration
      /// just using the multiply with the same value
			a2 = re * wre - im * wim;
			b2 = re * wim + im * wre;
			wre = a2;
			wim = b2;
		}
	}

	/// FFT separation and join (last stage of the real FFT)
	N = m_iSize;
	wre = re = cos(EAR_2PI / N);
	wim = im = sin(EAR_2PI / N);

	/// zero frequency
	_pData[0] += _pData[1];
	if(_pData[0] < 0) _pData[0] *= -1;

	/// do not use first element
	for (i = 2; i < m_iFFT; i += 2) {

    /// computing temp variables
		a1 = _pData[i] + _pData[m_iSize - i];
		b1 = (_pData[m_iSize - i] - _pData[i]) * wim - (_pData[i+1] + _pData[m_iSize - i + 1]) * wre;

		a2 = _pData[i+1] - _pData[m_iSize - i + 1];
		b2 = (_pData[m_iSize - i] - _pData[i]) * wre + (_pData[i+1] + _pData[m_iSize - i + 1]) * wim;

    /// the actual join and separation
		_pData[i] 							= (a1 - b1) / 2.0;
		_pData[m_iSize - i]			= (a1 + b1) / 2.0;

		_pData[i+1] 						= (b2 + a2) / 2.0;
		_pData[m_iSize - i + 1] = (b2 - a2) / 2.0;

    /// use recursion
		a2 = re * wre - im * wim;
		b2 = re * wim + im * wre;
		wre = a2;
		wim = b2;
	}

  /// compute modul of the complex values
	for(i=2, j=1; i<_pData.size(); i+=2, j++)
		_pData[j] = sqrt((_pData[i] * _pData[i]) + (_pData[i+1] * _pData[i+1]));

  /// the output is the half of input as we have real numbers right now
	_pData.size() /= 2;
	_pData.freq() /= 2;
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

  /// get new data
	m_Tmp.clear(); actualize(m_Tmp);
  /// empty, return empty container
	if(!m_Tmp.size()){_pData.size() =  0; return;}

  /// reserver space for otput
	_pData.reserve(m_iOutputSize); _pData.clear(); _pData.size() = m_iOutputSize;
  /// compute normalization factor
	norm = sqrt(2.0 / m_Tmp.size());
  /// if there is change of input size, reinitialize the transform matrix
	if(m_iInputSize != m_Tmp.size()) {initDct(m_Tmp.size());}

	/*
	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0; x = (float)(i+1) * EAR_PI / m_Tmp.size();
		for(j=0; j<m_Tmp.size(); j++)
			_pData[i] += m_Tmp[j] * cos(x*(1+j-0.5));

		_pData[i] *= norm;
	}*/

  /// multiply the vector with the matrix and normalization factor
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

  /// remove old matrix if initialized before
	if(m_pfCos)
	{
		for(i=0;i<m_iOutputSize;i++)
			delete[] m_pfCos[i];

		delete m_pfCos;
	}

  /// allocate new memory
	m_iInputSize = _iSize;
	m_pfCos = new float*[m_iOutputSize];

  /// compute the matrix
	for(i=0; i<m_iOutputSize; i++)
	{
		x = (float)(i+1) * EAR_PI / _iSize;
		m_pfCos[i] = new float[_iSize];
		for(j=0; j<_iSize; j++)
			m_pfCos[i][j] = cos(x*(1+j-0.5));
	}
}
