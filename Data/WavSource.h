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

 /**
 * This file contains basing reading of the audio WAV file. The file read needs to be
 * mono, 1,2 or 3 bytes per sample.
 */

#ifndef __EAR_WAVSOURCE_H_
#define __EAR_WAVSOURCE_H_

#include "Data.h"

namespace Ear
{
  /**
  * Reading input WAV file with limitations. The class can read only not compressed files, mono with 1 to 3 bytes per sample.
  * The class is reading whole file into memory.
  */
	class CWavSource : public ADataProcessor
	{
	public:
    /// Initialize reader
    /// @param [in] _fReadTime size of data read in one go.
		CWavSource(float _fReadTime);
		~CWavSource();

	private:
		unsigned int m_iSmpFreq;      ///< Sampling frequency read from file's header
		unsigned short m_iBytesPerSmp; ///< Bytes per samples read from file's header
		float m_fReadTime; ///< data length read in one go in seconds.
    unsigned int m_iReadLength; ///< data length read in one go in samples
    unsigned char *m_psBuf; ///< data read from file (samples)
    unsigned int m_iSize, m_iRead; ///< size of the samples read and read cursor

	public:
    /// Getting new data from processor
    /// @param [in] _pData Container to be filled with new data
    void getData(CDataContainer &_pData);
    /// Read WAV file
    /// @param [in] _szFileName name of the file to read
    /// @return success of the reading.
    unsigned int load(char *_szFileName);
	};
}

#endif
