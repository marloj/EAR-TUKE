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
 * This file contains the microphone read using portaudio library
 */

#ifndef __EAR_MICREADER_H_
#define __EAR_MICREADER_H_

#include "PushSource.h"
#include <portaudio.h>

namespace Ear {

    /**
     * Simple microhone read class. This class is using internally <i>PushSource</i> processor
     * for storing the samples.
     */
    class CMicSource : public ADataProcessor {
    public:
        /// Initialize microphone
        /// @param [in] _iBufferLength size of the internal sample buffer
        /// @param [in] _iReadLength length of the data to read when <i>getData</i> function is called
        /// @param [in] _iFreq sample frequency to request from operating system
        CMicSource(unsigned int _iBufferLength, unsigned int _iReadLength, int _iFreq);
        virtual ~CMicSource();

    private:
        CPushSource *m_pS; ///< internal processor for storing the samples
        int m_iFreq; ///< remembering sampling frequency
        PaStream *m_pStream; ///< port audio instance pointer

    public:
        /// Open microphone and try to start recording
        /// @return status of the opening attempt
        unsigned int open();
        /// Close microphone, stop recording
        void close();
        /// Get data read from microphone
        /// @param [in, out] _pData Container to be filled with data
        void getData(CDataContainer &_pData);
    };
}

#endif
