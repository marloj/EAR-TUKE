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

#ifndef __EAR_TRANSFORM_H_
#define __EAR_TRANSFORM_H_

#include "../Data/Data.h"

namespace Ear
{
	class CFourier : public ADataProcessor
	{
	public:
		CFourier();
		virtual ~CFourier();

	private:
		unsigned int m_iSize, m_iPower;

	public:
		void getData(CDataContainer &_pData);
	};

	class CDct : public ADataProcessor
	{
	public:
		CDct(unsigned int _iSize);
		virtual ~CDct();

	private:
		unsigned int m_iOutputSize, m_iInputSize;
		float **m_pfCos;
		CDataContainer m_Tmp;

	public:
		void getData(CDataContainer &_pData);

	private:
		void initDct(unsigned int _iSize);
	};
}

#endif
