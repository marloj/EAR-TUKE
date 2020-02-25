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
#include "MicSource.h"

using namespace Ear;

/// port audio callback funtion prototype defined here to be not available from outside
static int Callback(const void *inbuf, void *outbuf, unsigned long len, const PaStreamCallbackTimeInfo *outTime, PaStreamCallbackFlags statusFlags, void *userdata);

CMicSource::CMicSource(unsigned int _iBufferLength, unsigned int _iReadLength, int _iFreq)
{
    m_pStream = NULL;
    m_pS = NULL;
    /// new instance of the pushsource, which have circular buffer. Also we are passing the next parameter the number of samples read in one go.
    m_pS = new CPushSource(_iBufferLength * _iFreq, _iReadLength);
    /// also settings the sampling frequency of the push source.
    m_pS->changeFreq(_iFreq);
    m_iFreq = _iFreq;
}

CMicSource::~CMicSource()
{
    close();
    if (m_pS) {
        delete m_pS;
    }
}

unsigned int CMicSource::open()
{
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) {
        printf("Error: portaudio: failed to initialize: %s\n", Pa_GetErrorText(err));
        return EAR_FAIL;
    }

    err = Pa_OpenDefaultStream(&m_pStream, 1, 0, paInt16, m_iFreq, paFramesPerBufferUnspecified, Callback, m_pS);
    if (err != paNoError) {
        printf("Error: portaudio: error in opening stream: %s\n", Pa_GetErrorText(err));
        return EAR_FAIL;
    }

    err = Pa_StartStream(m_pStream);
    if (err != paNoError) {
        printf("Error: portaudio: failed to begin stream: %s\n", Pa_GetErrorText(err));
        m_pStream = NULL;
        return EAR_FAIL;
    }

    m_pS->openStream();

    return EAR_SUCCESS;
}

void CMicSource::close()
{
    PaError err;

    m_pS->closeStream();

    if (m_pStream == NULL) return;

    err = Pa_AbortStream(m_pStream);
    if (err != paNoError) {
        printf("Error: portaudio: failed to stop stream: %s\n", Pa_GetErrorText(err));
        return;
    }

    err = Pa_CloseStream(m_pStream);
    if (err != paNoError) {
        printf("Error: portaudio: failed to close stream: %s\n", Pa_GetErrorText(err));
        return;
    }

    err = Pa_Terminate();
    if (err != paNoError) {
        printf("Error: portaudio: failed to terminate library: %s\n", Pa_GetErrorText(err));
        return;
    }

    m_pStream = NULL;
}

void CMicSource::getData(CDataContainer &_pData)
{
    _pData.clear();
    if (m_pS) m_pS->getData(_pData);
}

int Callback(const void *inbuf, void *outbuf, unsigned long len, const PaStreamCallbackTimeInfo *outTime, PaStreamCallbackFlags statusFlags, void *userdata)
{
    CPushSource *ps = (CPushSource*) userdata; ///< the instance of this class is passed as userdata through portaudio
    int ret = 0;
    float sample = 0.0;

    /// convert the samples (2 byte short) to the float one by one and push to the buffer (pushsource)
    for (int i = 0; i < len; i++) {
        sample = ((short*) inbuf)[i];
        ret = ps->pushData(&sample, 1);
        if (ret == EAR_FAIL) {
            printf("MicSource: WARNING: buffer overflowed\n");
        }
    }

    return paContinue;
}
