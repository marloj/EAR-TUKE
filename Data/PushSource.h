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

#ifndef __EAR_PUSHSOURCE_H_
#define __EAR_PUSHSOURCE_H_

#include "Data.h"

namespace Ear
{
	class CPushSource : public ADataProcessor
	{
	public:
		CPushSource(unsigned int _iBufferLength, unsigned int _iReadLength);
		virtual ~CPushSource();

    private:
        //intern buffer variables
        float *m_pfBuf;
        unsigned int m_iRead, m_iWrite, m_iSize;

	private:
		unsigned int m_iReadLength, m_iFreq;
		bool m_bEndOfStream;

	public:
        unsigned int pushData(float *_pfData, unsigned int _iSize);
        void changeFreq(unsigned int _iFreq);
        void getData(CDataContainer &_pData);

        void openStream();
        void closeStream();
	};
}

#endif
