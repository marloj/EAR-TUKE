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


#include "Ear.h"
#include "Data/MicSource.h"

#define WAV_READ_CHUNK 1000

using Ear::App;
using Ear::Exception;
using Ear::CMicSource;

Exception::Exception(const std::string& msg) : _msg(msg)
{

}

const std::string& Exception::message() const{
    return _msg;   
}

App::App() : _audio(NULL)
{

}

App::~App()
{
    if (_audio != NULL) {
        delete _audio;
    }
}

void App::initModel()
{
    //load models and recognition network        
    char model_bin[PATH_MAX];
    char model_idx[PATH_MAX];
    _modelConfig.lookUpString("MODEL_IDX_FILE", model_idx, "model.idx");
    _modelConfig.lookUpString("MODEL_BIN_FILE", model_bin, "model.bin");

    unsigned int ret = _model.load(model_bin, model_idx);
    if (ret == EAR_FAIL) {
        throw Exception("Error reading model and idx file");
    }

    //create scorer for the search algorithm
    unsigned int strip = 0;
    _modelConfig.lookUpUInt("STRIP_OFFSET", &strip, 0);
    _scorer.setAcousticModel(_model.getAcousticData(), strip);

    //create search algorithm instance
    float insertionPenalty = 0;
    _modelConfig.lookUpFloat("INSERT_PENALTY", &insertionPenalty, -100);

    ret = _search.initialize(_model.getFSTData(), &_scorer, insertionPenalty);
    if (ret == EAR_FAIL) {
        throw Exception("Error creating search instance");
    }

    unsigned int mic_buffer = 8;
    int mic_freq = 16000;
    _modelConfig.lookUpUInt("MIC_BUFFER", &mic_buffer, 8);
    _modelConfig.lookUpInt("MIC_FREQ", &mic_freq, 16000);


    _audio = new CMicSource(WAV_READ_CHUNK, mic_buffer, mic_freq);
    ret = ((CMicSource *) _audio)->open();
    if (ret == EAR_FAIL) {
        throw Exception("Error initializing microphone");
    }

    //configuration for feature extraction
    _modelConfig.lookUpBool("ZERO_COEF", &_featureConfig.bC0, false);
    _modelConfig.lookUpBool("ENERGY", &_featureConfig.bEnergy, false);
    _modelConfig.lookUpBool("RAW_ENERGY", &_featureConfig.bRawE, false);
    _modelConfig.lookUpFloat("HAMMING", &_featureConfig.fHam, 0.46);
    _modelConfig.lookUpFloat("WND_LENGTH", &_featureConfig.fLength_ms, 25);
    _modelConfig.lookUpFloat("PREEM", &_featureConfig.fPreem, 0.97);
    _modelConfig.lookUpFloat("WND_SHIFT", &_featureConfig.fShift_ms, 10);
    _modelConfig.lookUpUInt("ACC_WND", &_featureConfig.iAccWin, 2);
    _modelConfig.lookUpUInt("CEP_NUM", &_featureConfig.iCep, 12);
    _modelConfig.lookUpUInt("DEL_WND", &_featureConfig.iDelWin, 2);
    _modelConfig.lookUpUInt("HI_FREQ", &_featureConfig.iHiFreq_hz, UINT_MAX);
    _modelConfig.lookUpUInt("LO_FREQ", &_featureConfig.iLoFreq_hz, 0);
    _modelConfig.lookUpUInt("LIFT_COEF", &_featureConfig.iLift, 22);
    _modelConfig.lookUpUInt("MEL_NUM", &_featureConfig.iMel, 29);
    _modelConfig.lookUpUInt("CMN_WND", &_featureConfig.iCMNWin, 0);
    char frn_type[100];
    _modelConfig.lookUpString("FRONT_END_TYPE", frn_type, "MFCC");
    if (strcmp(frn_type, "MFCC") == 0) {
        _featureConfig.iType = CFeature::Configuration::MFCC;
    }
    if (strcmp(frn_type, "MELSPEC") == 0) {
        _featureConfig.iType = CFeature::Configuration::MELSPEC;
    }
    if (strcmp(frn_type, "FBANK") == 0) {
        _featureConfig.iType = CFeature::Configuration::FBANK;
    }
    if (strcmp(frn_type, "DIRECT") == 0) {
        _featureConfig.iType = CFeature::Configuration::DIRECT;
    }

    //initialize front-end and set the audio source

    _feature.initialize(_featureConfig);
    _feature.setSource(_audio);
}

void App::loadConfig(const std::string& configFile)
{
    std::cout << "Loading configuration from " << configFile << std::endl;
    int ret = _modelConfig.load(configFile.c_str());
    if (ret != EAR_SUCCESS) {        
        throw Exception("Error reading configuration file");
    }
}

void App::run(const std::string& configFile)
{    
    loadConfig(configFile);

    initModel();

    int backgroundIndex = 1;
    int backgroundDuration = 10;

    //load some initial properties    
    _modelConfig.lookUpInt("BCG_IDX", &backgroundIndex, 1);
    _modelConfig.lookUpInt("BCG_DUR", &backgroundDuration, 10);
    //process all data
    int64_t iTime = 0;
    while (1) {
        CDataContainer data;
        //get new feature vector from front-end
        _feature.getData(data);
        if (data.size() == 0) {
            throw Exception("Got no data! (data.size==0)");
        }
        //process the one frame
        unsigned int ret = _search.process(data, iTime);
        iTime++;
        if (ret == EAR_FAIL) {
            throw Exception("Error in processing input data");
        }

        CResults results;
        //read the current results
        results.clear();
        _search.getResults(results);
        if (results.empty()) {
            continue;
        }
        CResult& last = results.back();
        //output them all
        if (last.iId == backgroundIndex && last.iDur > backgroundDuration) {
            for (CResults::iterator res = results.begin(); res != results.end(); res++) {
                //skip background output
                if (res->iId == backgroundIndex) {
                    continue;
                }
                float shift = (float) res->iDur * _featureConfig.fShift_ms / 1000;
                float score = res->fScore;
                double rms = data.rms();
                std::string model = std::string(_model.getDict()->ppszWords[res->iId]);
                std::cout << "RESULT;" << shift << ";" << score << ";" << rms << ";" << model << ";" << std::endl;
            }
            //resetting decoder in the background hypothesis
            //so the long term running of the system saves memory
            _search.reset();
        }
    }

}

int main(int argc, char* argv[])
{    
    try {
        std::string configFile;        
        if (argc>1){                       
            configFile = argv[1];
        } else {
            configFile = "ear.config";
        }
        App app;        
        app.run(configFile);        
    }
    catch (const Exception& ex) {              
        std::cerr << "error: " << ex.message() << std::endl;
        return -1;
    }
    catch (std::exception& ex) {
        std::cerr << "error: " << ex.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "error running !" << std::endl;
        return -1;
    }
    return 0;
}