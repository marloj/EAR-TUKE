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

#ifndef __EAR_FRAME_H_
#define __EAR_FRAME_H_

#include "../Data/Data.h"

namespace Ear
{
	class CFrame : public ADataProcessor
	{
	public:
		CFrame(float _fLength, float _fShift);
		virtual ~CFrame();

	private:
		float m_fLength, m_fShift;
		unsigned int m_iPos;
		CDataContainer bf1, bf2;

	public:
		void getData(CDataContainer &_pData);
	};
}

#endif
