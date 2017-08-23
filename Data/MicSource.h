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

#ifndef __EAR_MICREADER_H_
#define __EAR_MICREADER_H_

#include "PushSource.h"
#include <portaudio.h>

namespace Ear
{
	class CMicSource : public ADataProcessor
	{
	public:
		CMicSource(unsigned int _iBufferLength, unsigned int _iReadLength, int _iFreq);
		virtual ~CMicSource();

	private:
		CPushSource *m_pS;
		int m_iFreq;
		PaStream *m_pStream;

	public:
		unsigned int open();
		void close();
        void getData(CDataContainer &_pData);
	};
}

#endif
