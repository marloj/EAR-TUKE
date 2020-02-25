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
 * This file container definition of top level preprocessing class.
 */

#ifndef __EAR_FEATURE_H_
#define __EAR_FEATURE_H_

#include <limits.h>
#include <stdio.h>

namespace Ear {

    /**
     * Top level signal preprocessing class, computing features to enter the recognition/detection
     * process. This class is capable to create frontends with MFCC, MELSPEC and FBANK features.
     */
    class CFeature : public ADataProcessor {
    public:

        /**
         * Inner configuration class, containing default parameters, configuration variables
         */
        class Configuration {
        public:

            enum _FrontEndType_ {
                MELSPEC, ///< Mel-Spectrum coefficients
                FBANK, ///< F-Bank coefficients
                MFCC, ///< MFCCs
                DIRECT ///< direct input of coefficients from file (additional computation of CMN and delta coefficients is applicable)
            };

        public:
            /// Set default values for computation of the features

            Configuration() {
                fLength_ms = 25;
                fShift_ms = 10;
                fPreem = 0.97;
                fHam = 0.46;
                iLoFreq_hz = 0;
                iHiFreq_hz = UINT_MAX;
                iMel = 29;
                iCep = 12;
                iLift = 22;
                iAccWin = 2;
                iDelWin = 2;
                bRawE = 0;
                bC0 = 1;
                bEnergy = 0;
                iType = MFCC;
                iCMNWin = 0;
            }

        public:
            float fLength_ms; ///< window length
            float fShift_ms; ///< window shift
            float fPreem; ///< preemphasis coefficient
            float fHam; ///< hamming window coefficient
            unsigned int iLoFreq_hz; ///< low frequency limit for Mel-Bank filter
            unsigned int iHiFreq_hz; ///< hi frequency limit for Mel-Bank filter
            unsigned int iMel; ///< Number of Mel-Bank filters
            unsigned int iCep; ///< Number of cepstral coefficients after cosine transform
            unsigned int iLift; ///< filter factor in cepstral domain (lifter)
            unsigned int iAccWin; ///< Acceleration (delta order 2) coefficients window
            unsigned int iDelWin; ///< delta order 1 coefficient window
            unsigned int iCMNWin; ///< Cepstral mean normalization window length
            bool bRawE; ///< compute the energy before hamming window and preemphasis
            bool bC0; ///< compute the zero MFCC
            bool bEnergy; ///< compute energy coefficient
            unsigned int iType; ///< compute this type of features (MELSPEC, FBANK,  MFCC, DIRECT).
        };

    public:
        CFeature();
        virtual ~CFeature();

    public:
        /// Initialize whole front-end according settings
        /// @param [in] _cfg Configuration for the frontend preprocessing
        /// @return success of the initlialization
        unsigned int initialize(CFeature::Configuration &_cfg);
        /// Get new data from the whole preprocessing
        /// @param [in, out] _pData Container to be filled with new data
        void getData(CDataContainer &_pData);
        /// Set source of the data to be prerocessed. It can be the microphone, wav file, or already extracted coefficients
        /// @param [in] _pPrev pointer to the source processor
        void setSource(ADataProcessor *_pPrev);
        /// Returning last set processor source for this preprocessing
        /// @return pointer to the previous processor.
        ADataProcessor* getSource();

    private:
        ADataProcessor *m_pLast; ///< last processor in the processing chain. This is called for new data
        ADataProcessor *m_pFirst; ///< first processor in the processing chain. This is set with the source of new data

    private:
        /// Helper function to add new processor into the chain
        /// @param [in] _pProc new processor to add
        void addProcessor(ADataProcessor *_pProc);
    };
}

#endif
