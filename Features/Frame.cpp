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

#include "Frame.h"
//#include <stdio.h>

using namespace Ear;

CFrame::CFrame(float _fLength, float _fShift) : ADataProcessor()
{
	m_fLength = _fLength;
	m_fShift  = _fShift;
	m_iPos = 0;

	bf1.size() = 0; bf2.size() = 0;
}

CFrame::~CFrame()
{

}

void CFrame::getData(CDataContainer &_pData)
{
	unsigned int iLength, iShift;

	/// we are getting outside of the temporary buffer, so we need new data
  /// then we set the position to the begining of this buffer, thus zero
	if(m_iPos >= bf1.size()){bf1.clear(); actualize(bf1); m_iPos = 0;}
  /// we are getting empty container, return empty container
	if(!bf1.size()){_pData.clear(); return;}

	/// compute from milliseconds how many samples we need to copy
	iLength = (unsigned int)(bf1.freq() * m_fLength * 0.001);
	iShift  = (unsigned int)(bf1.freq() * m_fShift * 0.001);

	/// reserve space for output frame
	_pData.reserve(iLength); _pData.clear();

	/// copy old data we remembered from previous frame to output
  /// basically copy the overlap of previous frame
	if(bf2.size()) _pData.copy(&bf2,iShift,iLength);

	/// if more data are needed copy them from buf1 holding new data
	while(_pData.size() != iLength)
	{
    /// adjust the position in the buf1 and copy data
		m_iPos += _pData.add(&bf1,m_iPos, iLength - _pData.size());

    /// not enough data, we need to get more
		if(m_iPos >= bf1.size()){bf1.clear(); actualize(bf1); m_iPos = 0;}

    /// not enough data to read from source, stop trying
		if(!bf1.size()){break;}
	}

	//printf("%d\n", _pData.size());

	/// if bf1 has too low data do not create an another frame
  /// there is only 90% of frame filled, return empty container instead
	if(_pData.size() < iLength * 0.9){_pData.clear(); return;}

	/// set length to output data
	_pData.size() = iLength;

	/// copy the data to bf2 for remembering the overlap
	bf2.copy(&_pData);
}
