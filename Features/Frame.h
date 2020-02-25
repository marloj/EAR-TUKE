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
 * Each processing of the input signal is based on the dividing the input samples
 * into frames that overlap. This file contains processor for this purpose
 */

#ifndef __EAR_FRAME_H_
#define __EAR_FRAME_H_

#include "../Data/Data.h"

namespace Ear {

    /**
     * Class for dividing the input signal to specific length and overlap. The class is using
     * two temporary buffers. buf1 to read new data from source, as we do not know how many it will be
     * and buf2 for remembering last frame, to copy the overlap into next one.
     */
    class CFrame : public ADataProcessor {
    public:
        /// Initialize processor.
        /// @param [in] _fLength length of window in milliseconds
        /// @param [in] _fShift shift of the window in milliseconds
        CFrame(float _fLength, float _fShift);
        virtual ~CFrame();

    private:
        float m_fLength; ///< length of the window
        float m_fShift; ///< shift of the window
        unsigned int m_iPos; ///< position in the read input stream
        CDataContainer bf1, bf2; ///< two temporary containers

    public:
        void getData(CDataContainer &_pData);
    };
}

#endif
