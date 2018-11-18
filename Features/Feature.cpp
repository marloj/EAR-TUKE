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

#include <limits.h>
#include <stdio.h>

#include "../Data/Data.h"
#include "Frame.h"
#include "Feature.h"
#include "Transform.h"
#include "Filter.h"
#include "Coeffs.h"

using namespace Ear;

CFeature::CFeature() : ADataProcessor()
{
  /// no processor added
  m_pLast = NULL;
  m_pFirst = NULL;
}

CFeature::~CFeature()
{
  ADataProcessor *tmp = NULL;
	ADataProcessor *src = m_pFirst ? m_pFirst->getSource() : NULL;

  /// go through the processor chain and delete one after another.
  while(m_pLast && m_pLast != src)
  {
		tmp = m_pLast->getSource();
		delete m_pLast;
		m_pLast = tmp;
	}
}

void CFeature::setSource(ADataProcessor *_pPrev)
{
    m_pFirst->setSource(_pPrev);
}

ADataProcessor* CFeature::getSource()
{
    return m_pFirst->getSource();
}

unsigned int CFeature::initialize(Configuration &_cfg)
{
  /// if the frontend was initialized before, return fail
  if(m_pLast){/*printf("FrontEnd initialized, can not initialize again\n");*/ return EAR_FAIL;}

  /// helper variables (remembering pointers to concatenate the results at the end of processing)
  ADataProcessor *tmp = NULL; ///< last created processor
  AAuxDataProcessor *energy = NULL;
  AAuxDataProcessor *c0 = NULL;

	/// check the type of the features to compute
	if(_cfg.iType > 4) {/*printf("Wrong front-end type\n");*/ return EAR_FAIL;}

	/// if this is not DIRECT frontend, but all the computation is needed
  if(_cfg.iType != Configuration::DIRECT)
	{
	  /// Create frame from input signal
    tmp = new CFrame(_cfg.fLength_ms, _cfg.fShift_ms); addProcessor(tmp);
    /// Compute raw energy if desired
    if(_cfg.bEnergy && _cfg.bRawE) {energy = new CEnergy(); addProcessor(energy);}
    /// Compute preemphasis
    if(_cfg.fPreem > 0){tmp = new CPreem(_cfg.fPreem); addProcessor(tmp);}
    tmp = new CWindow(_cfg.fHam); addProcessor(tmp);
    /// Compute energy if needed
    if(_cfg.bEnergy && !_cfg.bRawE) {energy = new CEnergy(); addProcessor(energy);}
    /// Spectral analysis
    tmp = new CFourier(); addProcessor(tmp);
    /// Mel filter bank
    tmp = new CMelBank(_cfg.iLoFreq_hz, _cfg.iHiFreq_hz, _cfg.iMel, _cfg.iType != Configuration::MELSPEC); addProcessor(tmp);
    /// compute zero coefficent if required
    if(_cfg.bC0) {c0 = new CZeroCoef(); addProcessor(c0);}
    /// Ceptral analysis only for MFCC
    if(_cfg.iType == Configuration::MFCC)
    {
      /// Compute the cosine transform
      tmp = new CDct(_cfg.iCep); addProcessor(tmp);
      /// Lifter the signal
      tmp = new CLifter(_cfg.iLift); addProcessor(tmp);
    }

    /// Concatenate the zero cefficient and energy
    if(c0)		{tmp = new CConcat(); ((AAuxDataProcessor*)tmp)->setAuxSource(c0); addProcessor(tmp);}
    if(energy)	{tmp = new CConcat(); ((AAuxDataProcessor*)tmp)->setAuxSource(energy); addProcessor(tmp);}
	}

  /// Compute delta coeffs and acceleration
	if(_cfg.iDelWin) {tmp = new CDelta(_cfg.iDelWin,1); addProcessor(tmp); }
  if(_cfg.iDelWin && _cfg.iAccWin) {tmp = new CDelta(_cfg.iAccWin,2); addProcessor(tmp); }

  /// Compute cepstral mean normalization
  if(_cfg.iCMNWin != 0) {tmp = new CCMN(_cfg.iCMNWin); addProcessor(tmp); }

  /// return ok
	return EAR_SUCCESS;
}

void CFeature::getData(CDataContainer &_pData)
{
  /// ask last processor for new data
	m_pLast->getData(_pData);
	if(!_pData.size()){ _pData.clear(); return; }
}

void CFeature::addProcessor(ADataProcessor *_pProc)
{
  /// Set source of the processor to the last one, and rewrite the last pointer.
	_pProc->setSource(m_pLast); m_pLast = _pProc;
	if(m_pFirst == NULL){ m_pFirst = _pProc; }
}
