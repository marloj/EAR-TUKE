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
#include <string.h>

#include "Data/Data.h"
#include "Network/HTKAcousticModel.h"
#include "Network/Dictionary.h"
#include "Network/FSTAssembly.h"

using namespace Ear;

int main(int argc, char *argv[])
{
    CHTKAcousticModel model;
    CDictionary dict;
    CFSTAssembly fst;
    int ret = 0;

    char f[PATH_MAX], i[PATH_MAX], o[PATH_MAX], b[PATH_MAX], d[PATH_MAX];

    if (argc < 4) {
        printf("Usage: %s <htk model file> <dictionary> <output name prefix>", argv[0]);
        return 1;
    }

    ret = model.loadModelFrom(argv[1]);
    if (ret == EAR_FAIL) {
        printf("Error reading HTK acoustic model\n");
        return 1;
    }

    ret = dict.loadFromFile(argv[2]);
    if (ret == EAR_FAIL) {
        printf("Error reading dictionary file\n");
        return 1;
    }

    fst.assembly(&model, &dict);

    strcpy(f, argv[3]);
    strcat(f, ".fst");
    strcpy(i, argv[3]);
    strcat(i, ".isym");
    strcpy(o, argv[3]);
    strcat(o, ".osym");
    strcpy(b, argv[3]);
    strcat(b, ".bin");
    strcpy(d, argv[3]);
    strcat(d, ".idx");

    ret = fst.writeFST(f, i, o);
    if (ret == EAR_FAIL) {
        printf("Error writing output transducer\n");
        return 1;
    }

    ret = fst.writeBin(b, d);
    if (ret == EAR_FAIL) {
        printf("Error writing output binary model\n");
        return 1;
    }

    return 0;
}
