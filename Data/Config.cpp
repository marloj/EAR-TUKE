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

#include <string.h>

#include "Config.h"
#include "Utils.h"

using namespace Ear;
using namespace std;

CConfig::CConfig()
{
    //nothing to do here
}

CConfig::~CConfig()
{
    // clears the hash map of the properties
    m_mapConfig.clear();
}

unsigned int CConfig::load(const char *szFileName)
{
    FILE *pf = NULL;
    char szKey[100], szValue[100], szLine[500];
    int i = 0;

    pf = fopen(szFileName, "r");
    if(pf == NULL) return EAR_FAIL;

    while(fgets(szLine, 500, pf) != NULL)
    {
		if(szLine[0] == '#') continue;

        i = sscanf(szLine, "%s\t%s", szKey, szValue);
        if(i != 2) continue;

        m_mapConfig[szKey] = szValue;
    }

    fclose(pf);
    return EAR_SUCCESS;
}

void CConfig::print()
{
    map<string, string>::iterator it;
    printf("==============================================\n");

    for(it = m_mapConfig.begin(); it != m_mapConfig.end(); it++)
        printf("%s\t\t%s\n", it->first.c_str(), it->second.c_str());

    printf("==============================================\n");
}

const char *CConfig::findProperty(const char* _szPropertyName)
{
    map<string, string>::iterator it;
    string name(_szPropertyName);
    it = m_mapConfig.find(name);

    if(it != m_mapConfig.end()) return it->second.c_str();

    return NULL;
}

void CConfig::lookUpInt(const char *szPropertyName, int *_Value, int _Default)
{
    const char *szValue = findProperty(szPropertyName);
    *_Value = _Default;

    if(szValue != NULL) sscanf(szValue, "%d", _Value);
}

void CConfig::lookUpUInt  (const char *szPropertyName, unsigned int *_Value, unsigned int _Default)
{
    const char *szValue;

    *_Value = _Default;

    szValue = findProperty(szPropertyName);
    if(szValue != NULL) sscanf(szValue, "%u", _Value);
}

void CConfig::lookUpFloat (const char *szPropertyName, float *_Value, float _Default)
{
    const char *szValue;

    *_Value = _Default;

    szValue = findProperty(szPropertyName);
    if(szValue != NULL) sscanf(szValue, "%f", _Value);
}

void CConfig::lookUpDouble(const char *szPropertyName, double *_Value, double _Default)
{
    const char *szValue;

    *_Value = _Default;

    szValue = findProperty(szPropertyName);
    if(szValue != NULL) sscanf(szValue, "%lf", _Value);
}

void CConfig::lookUpString(const char *szPropertyName, char *_Value, const char *_Default)
{
    const char *szValue;

    strcpy(_Value, _Default);

    szValue = findProperty(szPropertyName);
    if(szValue != NULL) sscanf(szValue, "%s", _Value);
}

void CConfig::lookUpShort(const char *szPropertyName, short *_Value, short _Default)
{
    const char *szValue;

    *_Value = _Default;

    szValue = findProperty(szPropertyName);
    if(szValue != NULL) sscanf(szValue, "%hd", _Value);
}

void CConfig::lookUpBool(const char *szPropertyName, bool *_Value, bool _Default)
{
    const char *szValue;

    *_Value = _Default;

    szValue = findProperty(szPropertyName);
    if(szValue != NULL)
    {
        if(strcmp("T", szValue) == 0) *_Value = true;
        if(strcmp("F", szValue) == 0) *_Value = false;
    }
}
