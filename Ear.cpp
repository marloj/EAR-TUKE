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


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "Data/Data.h"
#include "Data/Config.h"
#include "Data/DataReader.h"
#include "Data/WavSource.h"
#include "Data/MicSource.h"
#include "Search/AcousticScorer.h"
#include "Search/Search.h"
#include "Features/Feature.h"

#define WAV_READ_CHUNK 1000

using namespace Ear;

int main(int argc, char* argv[])
{
    CConfig cfg;
    CFeature::Configuration fea_cfg;
    CFeature fea;
    char frn_type[100];
    char model_bin[PATH_MAX];
    char model_idx[PATH_MAX];
    ADataProcessor *audio;
    CAcousticScorer scorer;
    CDataHolder res;
    CSearch dec;
    CDataContainer data;
    CResults result;
    CResults::iterator it;
    int ret = 0;
    unsigned int strip = 0;
    float insertionPenalty = 0;
    int64_t iTime = 0;
    int bcg_id = 1;
    int bcg_dur = 10;
    bool online = false;
    unsigned int mic_buffer = 8;
    int mic_freq = 16000;

    if (argc < 1 && argc > 3) {
        fprintf(stderr, "Usage:\n\t%s <configuration file> <wav file>\n \t wav file processing", argv[0]);
        fprintf(stderr, "Usage:\n\t%s <configuration file>\n \t using a microphone input", argv[0]);
        return 1;
    }

    //load configuration file
    ret = cfg.load(argv[1]);
    if (ret == EAR_FAIL) {
        fprintf(stderr, "Error reading configuration file\n");
        return 1;
    }

    //cfg.print();

    //load some initial properties
    cfg.lookUpBool("ONLINE", &online, false);
    cfg.lookUpInt("BCG_IDX", &bcg_id, 1);
    cfg.lookUpInt("BCG_DUR", &bcg_dur, 10);

    //load models and recognition network
    cfg.lookUpString("MODEL_IDX_FILE", model_idx, "model.idx");
    cfg.lookUpString("MODEL_BIN_FILE", model_bin, "model.bin");
    ret = res.load(model_bin, model_idx);
    if (ret == EAR_FAIL) {
        fprintf(stderr, "Error reading model and idx file\n");
        return 1;
    }

    //create scorer for the search algorithm
    cfg.lookUpUInt("STRIP_OFFSET", &strip, 0);
    scorer.setAcousticModel(res.getAcousticData(), strip);

    //create search algorithm instance
    cfg.lookUpFloat("INSERT_PENALTY", &insertionPenalty, -100);
    ret = dec.initialize(res.getFSTData(), &scorer, insertionPenalty);
    if (ret == EAR_FAIL) {
        fprintf(stderr, "Error creating search instance\n");
        return 1;
    }

    //initialize audio source
    if (argc == 3) {
        audio = new CWavSource(WAV_READ_CHUNK);
        ret = ((CWavSource*) audio)->load(argv[2]); //load whole wav file
        if (ret == EAR_FAIL) {
            fprintf(stderr, "Error loading wav file from file %s\n", argv[2]);
        }
    }

    if (argc == 2) {
        cfg.lookUpUInt("MIC_BUFFER", &mic_buffer, 8);
        cfg.lookUpInt("MIC_FREQ", &mic_freq, 16000);
        audio = new CMicSource(WAV_READ_CHUNK, mic_buffer, mic_freq);
        ret = ((CMicSource*) audio)->open();
        if (ret == EAR_FAIL) {
            fprintf(stderr, "Error initializing microphone\n");
        }
    }

    //configuration for freature extraction
    cfg.lookUpBool("ZERO_COEF", &fea_cfg.bC0, false);
    cfg.lookUpBool("ENERGY", &fea_cfg.bEnergy, false);
    cfg.lookUpBool("RAW_ENERGY", &fea_cfg.bRawE, false);
    cfg.lookUpFloat("HAMMING", &fea_cfg.fHam, 0.46);
    cfg.lookUpFloat("WND_LENGTH", &fea_cfg.fLength_ms, 25);
    cfg.lookUpFloat("PREEM", &fea_cfg.fPreem, 0.97);
    cfg.lookUpFloat("WND_SHIFT", &fea_cfg.fShift_ms, 10);
    cfg.lookUpUInt("ACC_WND", &fea_cfg.iAccWin, 2);
    cfg.lookUpUInt("CEP_NUM", &fea_cfg.iCep, 12);
    cfg.lookUpUInt("DEL_WND", &fea_cfg.iDelWin, 2);
    cfg.lookUpUInt("HI_FREQ", &fea_cfg.iHiFreq_hz, UINT_MAX);
    cfg.lookUpUInt("LO_FREQ", &fea_cfg.iLoFreq_hz, 0);
    cfg.lookUpUInt("LIFT_COEF", &fea_cfg.iLift, 22);
    cfg.lookUpUInt("MEL_NUM", &fea_cfg.iMel, 29);
    cfg.lookUpUInt("CMN_WND", &fea_cfg.iCMNWin, 0);
    cfg.lookUpString("FRONT_END_TYPE", frn_type, "MFCC");
    if (strcmp(frn_type, "MFCC") == 0) fea_cfg.iType = CFeature::Configuration::MFCC;
    if (strcmp(frn_type, "MELSPEC") == 0) fea_cfg.iType = CFeature::Configuration::MELSPEC;
    if (strcmp(frn_type, "FBANK") == 0) fea_cfg.iType = CFeature::Configuration::FBANK;
    if (strcmp(frn_type, "DIRECT") == 0) fea_cfg.iType = CFeature::Configuration::DIRECT;

    //initialize frontend and set the wav source
    fea.initialize(fea_cfg);
    fea.setSource(audio);

    //process all data
    while (1) {
        //get new feature vector from frontend
        fea.getData(data);
        if (data.size() == 0) break;

        //process the one frame
        ret = dec.process(data, iTime);
        iTime++;
        printf("%10ld\r", iTime);
        if (ret == EAR_FAIL) {
            fprintf(stderr, "Error in processing input data\n");
            return 1;
        }

        //of online results are enabled 
        if (online) {

            //read the current results
            result.clear();
            dec.getResults(result);
            if (result.empty()) continue;
            CResult *r = &result.back();

            //output them all
            if (r->iId == bcg_id && r->iDur > bcg_dur) {
                for (it = result.begin(); it != result.end(); it++) {

                    //skip background output
                    if (it->iId == bcg_id) continue;

                    printf("%f\t%f\t%s\t%f\n", (float) it->iRevIndex * fea_cfg.fShift_ms / 1000,
                           (float) it->iDur * fea_cfg.fShift_ms / 1000,
                           res.getDict()->ppszWords[it->iId],
                           it->fScore);
                }

                //reseting decoder in the background hypothesis
                //so the long term runnig of the system saves memory
                dec.reset();
            }
        }
    }


    //display final results with the background
    if (!online) {
        result.clear();
        dec.getResults(result);

        printf("===================================results begin ========================================\n\n");
        for (it = result.begin(); it != result.end(); it++) {

            printf("%f\t%f\t%s\t%f\n", (float) it->iRevIndex * fea_cfg.fShift_ms / 1000,
                   (float) it->iDur * fea_cfg.fShift_ms / 1000,
                   res.getDict()->ppszWords[it->iId],
                   it->fScore);

        }
        printf("===================================results end ==========================================\n\n");
    }

    delete audio;

    return 0;

}

