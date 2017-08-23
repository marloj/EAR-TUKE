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

#ifndef __FILEIO_H_
#define __FILEIO_H_

#include <stdio.h>
#include <stdlib.h>
#include "Data.h"

#define MAX_BUF_LENGTH	5000

namespace Ear
{
	class FileIO
	{
	public:
		enum ioMode
		{
			READ,
			WRITE,
			WRITEBIN,
			READBIN
		};

	private:
		enum eState
		{
			READ_STATE,
			UNREAD_STATE
		};

	public:		
		FileIO();
		~FileIO();
		char *read();
		int open(const char *_szFileName, ioMode _iMode, const char *_szTokenizers);
		char *read(const char *_szExpect);
		char *skipTo(const char *_szExpect);
		int isOpen();
		void close();

	private:
		FILE *m_pf;
		char *readBuf;
		char *last;
		eState state;
		char *Toks;

	private:
		int readLine();
		void readyRead();
	};
}

#endif