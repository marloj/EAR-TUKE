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

class CConfig
{
  public:
     CConfig   ();
    ~CConfig   ();

  private:
    std::map<std::string, std::string> m_mapConfig;

  public:
    void lookUpInt   (const char *szPropertyName, int *_Value, int _Default);
    void lookUpUInt  (const char *szPropertyName, unsigned int *_Value, unsigned int _Default);
    void lookUpShort (const char *szPropertyName, short *_Value, short _Default);
    void lookUpFloat (const char *szPropertyName, float *_Value, float _Default);
    void lookUpDouble(const char *szPropertyName, double *_Value, double _Default);
    void lookUpString(const char *szPropertyName, char *_Value, const char *_Default);
    void lookUpBool  (const char *szPropertyName, bool *_Value, bool _Default);

  public:
    unsigned int load(const char *szFileName);
    void print();

  private:
    const char *findProperty(const char* _szPropertyName);

};
} //end of Éar namespace

#endif
