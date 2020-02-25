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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FileIO.h"

using namespace Ear;

FileIO::FileIO()
{
    m_pf = NULL;
    readBuf = new char[MAX_BUF_LENGTH];
    Toks = NULL;
}

int FileIO::open(const char *_szFileName, ioMode _ioMode, const char *_szTokenizers)
{
    m_pf = NULL;

    if (_ioMode == READ) m_pf = fopen(_szFileName, "r");
    if (_ioMode == WRITE) m_pf = fopen(_szFileName, "w");
    if (_ioMode == READBIN) m_pf = fopen(_szFileName, "rb");
    if (_ioMode == WRITEBIN) m_pf = fopen(_szFileName, "wb");

    if (m_pf == NULL) return EAR_FAIL;

    Toks = new char[strlen(_szTokenizers) + 1];
    strcpy(Toks, _szTokenizers);
    readyRead();

    return EAR_SUCCESS;
}

FileIO::~FileIO()
{
    if (m_pf != NULL) {
        fclose(m_pf);
        m_pf = NULL;
    }

    if (Toks != NULL) delete[] Toks;

    delete[] readBuf;
}

char *FileIO::read()
{
    if (last == NULL) return NULL;
    if (state == READ_STATE) {
        state = UNREAD_STATE;
        return last;
    } /// token already read

    /// get next token
    last = strtok(NULL, Toks);
    if (last == NULL) {
        if (readLine() == EAR_FAIL) return NULL; /// new line is needed to read
        last = strtok(readBuf, Toks);
    }

    return last;
}

char *FileIO::read(const char *_szExpect)
{
    if (state == UNREAD_STATE) {
        last = read();
        state = READ_STATE;
    } /// update the last token
    if (last == NULL) {
        state = UNREAD_STATE;
        return NULL;
    } /// no new token return NULL
    if (strcmp(last, _szExpect) == 0) {
        state = UNREAD_STATE;
        return last;
    } /// expecting token

    return NULL;
}

char *FileIO::skipTo(const char *_szExpect)
{
    state = UNREAD_STATE;
    if (last == NULL) last = read();
    while (last != NULL && strcmp(_szExpect, last) != 0) {
        last = read();
    } /// loop until the right token is found
    return last;
}

int FileIO::readLine()
{
    unsigned int length = 0;

    /// get whole line from file
    if (!fgets(readBuf, MAX_BUF_LENGTH, m_pf)) return EAR_FAIL;

    /// remove the new lines characters from the buffer
    length = strlen(readBuf);
    if (readBuf[length - 1] == '\n') readBuf[length - 1] = '\0';
    if (readBuf[length - 2] == '\r') readBuf[length - 2] = '\0';

    return EAR_SUCCESS;
}

int FileIO::isOpen()
{
    if (m_pf != NULL) return EAR_SUCCESS;

    return EAR_FAIL;
}

void FileIO::close()
{
    if (m_pf != NULL) {
        fclose(m_pf);
        m_pf = NULL;
    }
}

void FileIO::readyRead()
{
    /// initialize read
    last = NULL;

    /// try to read first line and start tokenizer
    if (readLine() == EAR_SUCCESS) last = strtok(readBuf, Toks);
    state = READ_STATE;
}
