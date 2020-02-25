/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Ear.h
 * Author: danny
 *
 * Created on February 25, 2020, 12:01 PM
 */

#ifndef EAR_H
#define EAR_H

#include "Data/DataReader.h"
#include "Data/Config.h"
#include "Search/Search.h"
#include "Features/Feature.h"
#include <exception>
#include <iostream>
#include <string>

namespace Ear {

    class Exception : public std::exception {
    public:
        Exception(const std::string& msg);
        const std::string& message() const;
    private:
        std::string _msg;
    };

    class App {
    public:
        App();
        ~App();

        void run(const std::string& configFile);
    private:
        void initModel();
        void loadConfig(const std::string& configFile);

        CDataHolder _model;
        CConfig _modelConfig;
        CSearch _search;
        CFeature _feature;
        CFeature::Configuration _featureConfig;
        CAcousticScorer _scorer;
        ADataProcessor *_audio;
    };

}
#endif /* EAR_H */

