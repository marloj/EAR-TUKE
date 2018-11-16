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
 * This file contains definition of the class for reading configuration files
 * This file does not contains default values for the configuration
 */

#ifndef __EARCONFIG_H_
#define __EARCONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <map>

#include "Data.h"

namespace Ear
{

/**
* Class for reading, parsing configuration file
*/
class CConfig
{
  public:
     CConfig   ();
    ~CConfig   ();

  private:
    /// hash map of the configuration parsed from file
    std::map<std::string, std::string> m_mapConfig;

  public:
    /**
    * Search for the property name, cast it to <b>int</b> and write into <b>_Value</b>
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpInt   (const char *szPropertyName, int *_Value, int _Default);
    /**
    * Search for the property name, cast it to <b>unsigned int</b> and write into <b>_Value</b>
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpUInt  (const char *szPropertyName, unsigned int *_Value, unsigned int _Default);
    /**
    * Search for the property name, cast it to <b>short</b> and write into <b>_Value</b>
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpShort (const char *szPropertyName, short *_Value, short _Default);
    /**
    * Search for the property name, cast it to <b>float</b> and write into <b>_Value</b>
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpFloat (const char *szPropertyName, float *_Value, float _Default);
    /**
    * Search for the property name, cast it to <b>double</b> and write into <b>_Value</b>
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpDouble(const char *szPropertyName, double *_Value, double _Default);
    /**
    * Search for the property name, cast it to <b>char string</b> and write into <b>_Value</b>
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpString(const char *szPropertyName, char *_Value, const char *_Default);
    /**
    * Search for the property name, cast it to <b>boolean</b> and write into <b>_Value</b>.
    * The function accepts chars 'T' or 'F' to define true or false respectively
    * @param [in] szPropertyName name of the searched property
    * @param [in, out] _Value pointer to the variable to set according configurated property
    * @param [in] _Default the default variable to be passed to the _Value if the property is not found
    */
    void lookUpBool  (const char *szPropertyName, bool *_Value, bool _Default);

  public:
    /**
    * Loads the configuration file
    * @params [in] szFileName path to the configuration file
    */
    unsigned int load(const char *szFileName);
    /**
    * Prints the configuration set by the config file to standard output
    */
    void print();

  private:
    /**
    * finds configuration property
    * @params [in] _szPropertyName property name to find
    * @return string value of the configuration property
    */
    const char *findProperty(const char* _szPropertyName);

};
} //end of Éar namespace

#endif
