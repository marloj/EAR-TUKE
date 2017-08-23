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

#ifndef __EAR_WAVSOURCE_H_
#define __EAR_WAVSOURCE_H_

#include "Data.h"

namespace Ear
{
	class CWavSource : public ADataProcessor
	{
	public:
		CWavSource(float _fReadTime);
		~CWavSource();

	private:
		//properties of wav file read from file
		unsigned int m_iSmpFreq;
		unsigned short m_iBytesPerSmp;

		//properties for reading of the frontend
		float m_fReadTime; unsigned int m_iReadLength;

		//buffer for storing entire wav file
        unsigned char *m_psBuf; unsigned int m_iSize, m_iRead;

	public:
        void getData(CDataContainer &_pData);
        unsigned int load(char *_szFileName);
        //void reset();
	};
}

#endif
