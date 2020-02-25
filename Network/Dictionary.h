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
 * Dictionary implementation. This is part of the conversion tool from HTK format
 * of acoustic models to native binary and for building the finite state transducer
 * network.
 */

#ifndef __DICTIONARYLOADER_H_
#define __DICTIONARYLOADER_H_

#include <string.h>
#include "../Data/FileIO.h"

namespace Ear {

    /**
     * Structure of one word (event name) in dictionary. This is dictionary definition is separate
     * from the one used in event detection. This implementation is used for reading the dictionary
     * for conversion to the finite-state transducer network.
     * This is part of the conversion tool from HTK format to native binary EAR-TUKE format
     * The dictionary is in following format:
     * <output event name> <event probability> <model name>
     */
    typedef struct DictItem_ {
        char *word; ///< output event name
        unsigned int id; ///< id of the event
        float prob; ///< event probability
        char *models; ///< model belonging to the event
        DictItem_ *next; ///< next event (this is linked list)
    } DictItem;

    class CDictionary {
    public:
        CDictionary();
        ~CDictionary();
        /// Loading dictionary from file
        /// @param [in] _szFileName dictionary to load
        int loadFromFile(char *_szFileName);
        /// Return pointer to the first item in the linked list of events
        DictItem *getDict();

    private:
        /// Adding new item to the linked list of events
        /// @param [in] _item new item to add
        void add(DictItem *_item);

        /// operator for searching in the map structure as we will use array of chars later

        struct cmp_str {

            bool operator()(char const *a, char const *b) const {
                return strcmp(a, b) < 0;
            }
        };

    private:
        FileIO m_file; ///< opened dictionary file
        DictItem *m_Dic; ///< events linked list
    };
}

#endif
