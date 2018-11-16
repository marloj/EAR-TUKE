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
 * This file contains pushsource definition being basicaly a circular buffer
 * not using any mutexes.
 */

#ifndef __EAR_PUSHSOURCE_H_
#define __EAR_PUSHSOURCE_H_

#include "Data.h"

namespace Ear
{
  /**
  * The PushSource is circular buffer. Normaly the design of the preprocessing processors
  * is to request data from previous one. In some cases, like reading from microphone, we need
  * to have a buffer in between while the microphone library is pushing data to us not waiting for us to read them
  * Circular buffer is using floats data type.
  */
	class CPushSource : public ADataProcessor
	{
	public:
    /// Initialize the circular buffer
    /// @param [in] _iBufferLength length of the circular buffer
    /// @param [in] _iReadLength length of the data to read in one function call (request)
		CPushSource(unsigned int _iBufferLength, unsigned int _iReadLength);
		virtual ~CPushSource();

  private:
    float *m_pfBuf; ///< internal array for the circular buffer
    unsigned int m_iRead, m_iWrite, m_iSize; ///< Read, Write cursors and Size of the allocated buffer.

	private:
		unsigned int m_iReadLength, m_iFreq;  ///< Remembering the langth of the data to read and sampling frequency
		bool m_bEndOfStream;  ///< Inication of the end of stream (external source is settings this when no more data will be available so we can pass this on.)

	public:
    /// Function for pushing data to the circular buffer. The buffer can overflow if pushed data has larger size as free space available.
    /// this case only data that are fitting into free space are inserted, the other discarted.
    /// @param [in] _pfData data to push in.
    /// @param [in] _iSize size of the data to push in.
    /// @return success of the pushed data into buffer (EAR_FAIL) in case of overflowing the buffer.
    unsigned int pushData(float *_pfData, unsigned int _iSize);
    /// Changing frequency of the input data, so we can pass this information to the output data containers.
    /// @param [in] _iFreq frequency of the input samples
    void changeFreq(unsigned int _iFreq);
    /// Getting new data from the buffer
    /// @param [in, out] _pData Container to fill with the new data
    void getData(CDataContainer &_pData);
    /// Indicating that stream is opening and there are data available
    void openStream();
    /// Indicating that there will be no more data available.
    void closeStream();
	};
}

#endif
