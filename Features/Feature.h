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

#ifndef __EAR_FEATURE_H_
#define __EAR_FEATURE_H_

#include <limits.h>
#include <stdio.h>

namespace Ear
{
	class CFeature : public ADataProcessor
	{
    public:
        class Configuration
        {
        public:
            enum _FrontEndType_ {  MELSPEC, FBANK,  MFCC, DIRECT };

        public:
            Configuration(){
                                fLength_ms = 25; fShift_ms = 10; fPreem = 0.97;
                                fHam = 0.46; iLoFreq_hz = 0; iHiFreq_hz = UINT_MAX;
                                iMel = 29; iCep = 12; iLift = 22; iAccWin = 2; iDelWin = 2;
                                bRawE = 0; bC0 = 1; bEnergy = 0;
                                iType = MFCC; iCMNWin = 0;
                            }

        public:
            float fLength_ms, fShift_ms, fPreem, fHam;
            unsigned int iLoFreq_hz, iHiFreq_hz, iMel, iCep, iLift, iAccWin, iDelWin, iCMNWin;
            bool bRawE, bC0, bEnergy;
            unsigned int iType;
        };


	public:
		CFeature();
		virtual ~CFeature();

	public:
		unsigned int initialize(CFeature::Configuration &_cfg);
		void getData(CDataContainer &_pData);
        void setSource(ADataProcessor *_pPrev);
		ADataProcessor* getSource();

	private:
		ADataProcessor *m_pLast, *m_pFirst;

	private:
		void addProcessor(ADataProcessor *_pProc);
    };
}

#endif
