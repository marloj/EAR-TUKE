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
#include "Data/WavSource.h"
#include "Features/Feature.h"

#define WAV_READ_CHUNK 1000

using namespace Ear;

int main(int argc, char *argv[])
{
    CConfig cfg;
    CFeature::Configuration fea_cfg;
    CFeature fea;
    ADataProcessor *audio;
    char frn_type[100];
    char model_bin[PATH_MAX];
    char model_idx[PATH_MAX];
    CDataContainer data;
    float insertionPenalty = 0;
    int64_t iTime = 0;
    int ret = 0;
    FILE *pOut = NULL;

    if (argc != 4) {
        printf("Usage:\n\t%s <configuration file> <wav file> <out file>\t wav file processing\n", argv[0]);
        return 1;
    }

    //load configuration file
    ret = cfg.load(argv[1]);
    if (ret == EAR_FAIL) {
        printf("Error reading configuration file\n");
        return 1;
    }

    //initialize audio source
    audio = new CWavSource(WAV_READ_CHUNK);
    ret = ((CWavSource *) audio)->load(argv[2]); //load whole wav file
    if (ret == EAR_FAIL) {
        printf("Error loading wav file from file %s\n", argv[2]);
        return 1;
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
    if (strcmp(frn_type, "MFCC") == 0)
        fea_cfg.iType = CFeature::Configuration::MFCC;
    if (strcmp(frn_type, "MELSPEC") == 0)
        fea_cfg.iType = CFeature::Configuration::MELSPEC;
    if (strcmp(frn_type, "FBANK") == 0)
        fea_cfg.iType = CFeature::Configuration::FBANK;
    if (strcmp(frn_type, "DIRECT") == 0)
        fea_cfg.iType = CFeature::Configuration::DIRECT;

    //initialize frontend and set the wav source
    fea.initialize(fea_cfg);
    fea.setSource(audio);

    //open output file
    pOut = fopen(argv[3], "wb");
    if (pOut == NULL) {
        printf("Error opening output file %s\n", argv[3]);
        return 1;
    }

    //define htk feature header

    typedef struct
    {
        unsigned int nSamples;
        unsigned int sampPeriod;
        unsigned short sampSize;
        unsigned short parmKind;
    } htk_header;

    union
    {
        htk_header header;
        unsigned char byte[12];
    } union_header;

    union
    {
        float number;
        unsigned char byte[4];
    } conv_float;

    //initilize the header
    union_header.header.nSamples = 0;
    union_header.header.sampPeriod = fea_cfg.fShift_ms * 10000;
    if (fea_cfg.iType == CFeature::Configuration::MFCC)
        union_header.header.parmKind = 6;
    if (fea_cfg.iType == CFeature::Configuration::FBANK)
        union_header.header.parmKind = 7;
    if (fea_cfg.iType == CFeature::Configuration::MELSPEC)
        union_header.header.parmKind = 8;
    if (fea_cfg.bEnergy)
        union_header.header.parmKind += 64;
    if (fea_cfg.bC0)
        union_header.header.parmKind += 8192;
    if (fea_cfg.iAccWin != 0)
        union_header.header.parmKind += 512;
    if (fea_cfg.iDelWin != 0)
        union_header.header.parmKind += 256;

    //write dump header
    fwrite(&union_header.byte, 1, 12, pOut);

    //process all data
    while (1) {
        //get new feature vector from frontend
        fea.getData(data);
        if (data.size() == 0)
            break;

        //write output to the file
        for (int i = 0; i < data.size(); i++) {
            conv_float.number = data[i];
            fwrite(&conv_float.byte[3], 1, 1, pOut);
            fwrite(&conv_float.byte[2], 1, 1, pOut);
            fwrite(&conv_float.byte[1], 1, 1, pOut);
            fwrite(&conv_float.byte[0], 1, 1, pOut);
        }

        union_header.header.nSamples++;
        union_header.header.sampSize = data.size();
    }

    union_header.header.sampSize *= sizeof (float);

    //write header of the feature file
    fseek(pOut, 0, SEEK_SET);
    fwrite(&union_header.byte[3], 1, 1, pOut);
    fwrite(&union_header.byte[2], 1, 1, pOut);
    fwrite(&union_header.byte[1], 1, 1, pOut);
    fwrite(&union_header.byte[0], 1, 1, pOut);
    fwrite(&union_header.byte[7], 1, 1, pOut);
    fwrite(&union_header.byte[6], 1, 1, pOut);
    fwrite(&union_header.byte[5], 1, 1, pOut);
    fwrite(&union_header.byte[4], 1, 1, pOut);
    fwrite(&union_header.byte[9], 1, 1, pOut);
    fwrite(&union_header.byte[8], 1, 1, pOut);
    fwrite(&union_header.byte[11], 1, 1, pOut);
    fwrite(&union_header.byte[10], 1, 1, pOut);

    fclose(pOut);

    delete audio;

    return 0;
}
