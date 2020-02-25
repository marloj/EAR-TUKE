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
 *  This file contains file input output definition used for parsing and reading text files
 */

#ifndef __FILEIO_H_
#define __FILEIO_H_

#include <stdio.h>
#include <stdlib.h>
#include "Data.h"

#define MAX_BUF_LENGTH 5000

namespace Ear {

    /**
     * Convenient class for reading text files and parsing. This is mainly used for reading HTK acoustic models.
     * This class uses <i>strtok</i>, which is not getting along with optimization O1 through O6, so this needs to be compiled
     * with no optimization flag O0!
     */
    class FileIO {
    public:
        /// IO modes of the opening file

        enum ioMode {
            READ,
            WRITE,
            WRITEBIN,
            READBIN
        };

    private:
        /// State of the reading process. Reading process uses internal variable <i>last</i> holding last read token,
        /// which may or may not be processed yet, this states are for indicating when the variable needs to be updated

        enum eState {
            READ_STATE,
            UNREAD_STATE
        };

    public:
        FileIO();
        ~FileIO();
        /// Open file with selected mode and tokenizer characters. Each call to read later will
        /// stop according the tokenizer characters.
        /// @param [in] _szFileName file name to open
        /// @param [in] _iMode mode of the file to open
        /// @param [in] _szTokenizers the list of tokenizing characters
        /// @return success of the opening
        int open(const char *_szFileName, ioMode _iMode, const char *_szTokenizers);
        /// Close the opened file
        void close();
        /// Function for reading next token from the file using the tokenizing characters
        char *read();
        /// Read expected token
        /// @param [in] _szExpect expected token to read from file
        /// @return expected read token or NULL if no such token was found in next read
        char *read(const char *_szExpect);
        /// Read everything until the expected token
        /// @param [in] _szExpect expected token to read into
        /// @return expected token if found, otherwise NULL if end of file is reached and the token not found
        char *skipTo(const char *_szExpect);
        /// @return true if file is currently opened.
        int isOpen();

    private:
        FILE *m_pf; ///< file handler pointer
        char *readBuf; ///< buffer holding one line from file
        char *last; ///< buffer holding the last token read
        eState state; ///< current state of the reading process
        char *Toks; ///< tokenizing characters

    private:
        /// Read whole line from file
        /// @return success of the process
        int readLine();
        /// Using for reading whole line from file and preparing strtok.
        void readyRead();
    };
}

#endif
