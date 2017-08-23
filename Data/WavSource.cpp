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
#include <string.h>
#include "WavSource.h"
#include "Utils.h"

using namespace Ear;

CWavSource::CWavSource(float _fReadTime) : ADataProcessor()
{
    m_fReadTime = _fReadTime;
    m_iBytesPerSmp = 0;
    m_iSmpFreq = 0;
    m_iSize = 0;
    m_iReadLength = 0;
    m_psBuf = NULL;
    m_iRead = 0;
}

CWavSource::~CWavSource()
{
    if(m_psBuf){ delete[] m_psBuf; }
}

unsigned int CWavSource::load(char *_szFileName)
{
    //variable for file pointer
    FILE *pf = NULL; unsigned short iT; int c;

    //open wav file
    pf = fopen(_szFileName, "rb");
    if(!pf) {/*printf("CWavSource: Error opening input file %s", _szFileName);*/ return EAR_FAIL;}

    //read header
    while((c = getc(pf)) != EOF)
	{
		if(c != 'f') continue; if((c = getc(pf)) == EOF) return EAR_FAIL;
		if(c != 'm') continue; if((c = getc(pf)) == EOF) return EAR_FAIL;
		if(c != 't') continue; if((c = getc(pf)) == EOF) return EAR_FAIL;
		if(c != ' ') continue;

		fseek(pf, 4, SEEK_CUR);

		fread(&iT, 2, 1, pf); if(iT != 1) {/*printf("CWavSource: This Audio File is Compressed, Compression is not Supported\n");*/ return EAR_FAIL;}
		fread(&iT, 2, 1, pf); if(iT != 1) {/*printf("CWavSource: This Audio has more than one chanel, this is not supported\n");*/ return EAR_FAIL;}
		fread(&m_iSmpFreq, 4, 1, pf);

		fseek(pf, 6, SEEK_CUR);

		fread(&m_iBytesPerSmp, 2, 1, pf); m_iBytesPerSmp /= 8;
		if(m_iBytesPerSmp != 2 && m_iBytesPerSmp != 1 && m_iBytesPerSmp != 3) {/*printf("Not supported number of bits per sample %d\n", m_iBytesPerSmp * 8);*/ return EAR_FAIL;}
		break;
	}

	//skip to data
	while((c = getc(pf)) != EOF)
	{
		if(c != 'd') continue; if((c = getc(pf)) == EOF) return EAR_FAIL;
		if(c != 'a') continue; if((c = getc(pf)) == EOF) return EAR_FAIL;
		if(c != 't') continue; if((c = getc(pf)) == EOF) return EAR_FAIL;
		if(c != 'a') continue;

		fseek(pf, 4, SEEK_CUR);
		break;
	}

    //get data length from size of whole file
    unsigned int iPos = ftell(pf); fseek(pf, 0, SEEK_END);
    m_iSize = (unsigned int)ftell(pf) - iPos;
    fseek(pf, iPos, SEEK_SET);

    //allocate data buffer and read all samples
    m_psBuf = new unsigned char[m_iSize];
    fread(m_psBuf, 1, m_iSize, pf);

    //close file
    fclose(pf);

    //compute read length for this file and reset read
    m_iReadLength = m_iSmpFreq * m_fReadTime * m_iBytesPerSmp;
    //reset();

    return EAR_SUCCESS;
}

void CWavSource::getData(CDataContainer &_pData)
{
    //compute available data
    unsigned int iAvail = m_iSize - m_iRead;
    if(iAvail == 0) { _pData.clear(); return; }

    //compute length of data to be copied
    if(iAvail > m_iReadLength){ iAvail = m_iReadLength; }
    iAvail /= m_iBytesPerSmp; _pData.reserve(iAvail);

    //compute start pointers
    unsigned int i = 0;
    float *pDst = _pData.data(); unsigned char *pSrc = m_psBuf + m_iRead;

    //transform and copy data
    if(m_iBytesPerSmp == 1)
    {
	    for(i = 0; i < iAvail; i++) { pDst[i] = ((float)pSrc[i] - 128); }
    }

    if(m_iBytesPerSmp == 2)
    {
        for(i = 0; i < iAvail; i++){ pDst[i] = ((float)((short*)pSrc)[i]); }
    }

    if(m_iBytesPerSmp == 3)
    {
        int smp; //float scale = (float)0x8000/(float)0x80000000;  //scaling to range of 16-bit samples

        for(i = 0; i < iAvail; i++)
        {
            smp = pSrc[i*3] << 8 | pSrc[i*3+1] << 16 | pSrc[i*3+2] << 24;
            pDst[i] = ((float)smp / 256.0);
        }
    }

    _pData.size() = iAvail;
    _pData.freq() = m_iSmpFreq;

    m_iRead += iAvail * m_iBytesPerSmp;
}
