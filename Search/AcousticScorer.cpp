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
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

#include "AcousticScorer.h"
#include "../Data/Data.h"

using namespace Ear;

CAcousticScorer::CAcousticScorer()
{
	am = NULL;
	scores = NULL;
	vector = NULL;
}

CAcousticScorer::~CAcousticScorer()
{
	if(scores != NULL) delete[] scores;
}

void CAcousticScorer::setAcousticModel(EAR_AM_Info *_am, unsigned int _iStrip_offset)
{
	//set acoustic model
	am = _am;

	m_iStrip_offset = _iStrip_offset;

	//allocate array for caching
	scores = new float[am->iNumberOfStates];
	memset(scores, 0.0, sizeof(float) * am->iNumberOfStates);
}

int CAcousticScorer::set(CDataContainer *_vector)
{
	if(_vector->size() != am->iVectorSize) {
		fprintf(stderr, "AcousticScorer: Incompatible features: model: %d, input: %d\n", _vector->size(), am->iVectorSize);
		return EAR_FAIL;
	}

	vector  = _vector;
	memset(scores, 0.0, sizeof(float) * am->iNumberOfStates);

	return EAR_SUCCESS;
}

float CAcousticScorer::getScore(unsigned int _Index)
{
	unsigned int i,j;
	unsigned int *pdfs;
	unsigned int Index = _Index - 1;
	EAR_AM_Pdf *pdf = NULL;
	float score = -1.0E10;
	float sx	= 0.0;
	float xmu	= 0.0;

	//check if the score was not computed for this state
	if(scores[Index] != 0.0) return scores[Index];

	//get pdfs to compute
	pdfs = am->States[Index];

	//go through pdfs and compute score
	for(i=0;i<am->iPdfsOnState;i++)
	{
		//if current pdf is not available take next one
		if(pdfs[i] == NONE) continue;

		//get current pdf to compute
		pdf = &(am->Pdfs[pdfs[i]-1]);

		//null working variables
		xmu = 0.0; sx = 0.0;

		sx += pdf->fgconst;

		//compute pdf
		for(j=m_iStrip_offset;j<am->iVectorSize;j++)
		{
			xmu = (*(vector))[j] - pdf->fMean[j];
			sx += xmu * xmu * pdf->fVar[j];
		}

        sx *= -0.5;

        sx += log(pdf->fWeight);

		if(sx > score) { score = sx; }
    }

	//register computed score
	scores[Index] = score;

	//return computed score
	return score;
}
