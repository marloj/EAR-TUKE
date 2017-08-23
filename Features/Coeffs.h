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

#ifndef __EAR_COEFFS_H_
#define __EAR_COEFFS_H_

#include "../Data/Data.h"

namespace Ear
{
	class CDelta : public ADataProcessor
	{
	public:
		CDelta(unsigned int _iWin, unsigned int m_iOrd);
		virtual ~CDelta();

	private:
		CDataContainer **m_pBuffer;
		unsigned int iDummy, m_iWin, m_iBf, m_iSize, m_iOrd;

	public:
		void getData(CDataContainer &_pData);

	private:
		void rotate();
	};

	class CEnergy : public AAuxDataProcessor
	{
	public:
		CEnergy();
		virtual ~CEnergy();

	public:
		void getData(CDataContainer &_pData);
		void getAuxData(CDataContainer &_pData);

	private:
		float m_fEnergy;
	};

	class CZeroCoef : public AAuxDataProcessor
	{
	public:
		CZeroCoef();
		virtual ~CZeroCoef();

	public:
		void getData(CDataContainer &_pData);
		void getAuxData(CDataContainer &_pData);

	private:
		float m_fC0;
	};
}

#endif
