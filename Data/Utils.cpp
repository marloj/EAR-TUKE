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

#include "Utils.h"

#include <string.h>
#include <stdio.h>

char *Ear::cloneString(char *_c)
{
	char *o;
	if(_c == NULL) return NULL;
	o = new char[strlen(_c) + 1];
	strcpy(o, _c);
	return o;
}

char *Ear::parse(char *_s)
{
	return strtok(_s, " ");
}

char *Ear::parseNext()
{
	return strtok(NULL, " ");
}

float *Ear::cloneVector(float *_f, unsigned int _x)
{
	float *o = NULL;
	if(_f == NULL) return NULL;
	o = new float[_x];
	memcpy(o, _f, sizeof(float) * _x);
	return o;
}
