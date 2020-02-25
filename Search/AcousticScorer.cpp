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
    /// deleting the cache memory for already computed scores
    if (scores != NULL)
        delete[] scores;
}

void CAcousticScorer::setAcousticModel(EAR_AM_Info *_am, unsigned int _iStrip_offset)
{
    /// set acoustic model
    am = _am;
    /// set strip offset
    m_iStrip_offset = _iStrip_offset;

    /// allocate memory for the score cache
    scores = new float[am->iNumberOfStates];
    /// reset the memory
    memset(scores, 0.0, sizeof (float) * am->iNumberOfStates);
}

int CAcousticScorer::set(CDataContainer *_vector)
{
    /// check vector size, it needs to be equal to the model used
    if (_vector->size() != am->iVectorSize) {
        printf("AcousticScorer: Incompatible features: model: %d, input: %d\n", _vector->size(), am->iVectorSize);
        return EAR_FAIL;
    }

    /// set current vector;
    vector = _vector;

    /// reset the score cache
    memset(scores, 0.0, sizeof (float) * am->iNumberOfStates);

    return EAR_SUCCESS;
}

float CAcousticScorer::getScore(unsigned int _Index)
{
    unsigned int i, j;
    unsigned int *pdfs;
    unsigned int Index = _Index - 1; /// as zero is reserved for empty symbol the indexes are shifted by one. The actual index of state in ascoustic model is less by one
    EAR_AM_Pdf *pdf = NULL;
    float score = -1.0E10; /// holding maximum score of the computed from each individual PDFs
    float sx = 0.0;
    float xmu = 0.0;

    /// check if the score was already computed, if so, return the cached value
    if (scores[Index] != 0.0)
        return scores[Index];

    /// get PDFs of the state to compute the score
    pdfs = am->States[Index];

    /// go through PDFs and compute the overall score
    for (i = 0; i < am->iPdfsOnState; i++) {
        /// if the PDF does not exists, skip and go to next one
        /// this can happen as the model can have less PDFs for some state as originally stated
        /// if the model was trained with HTK toolkit, the toolkit tends to drop some inefficient PDFs
        /// and not include them to the resulting models.
        if (pdfs[i] == NONE)
            continue;

        /// get the PDF information for computing the score
        pdf = &(am->Pdfs[pdfs[i] - 1]);

        /// prepare working variables
        xmu = 0.0;
        sx = 0.0;

        /// include precomputed value of the PDF
        sx += pdf->fgconst;

        /// compute the PDF
        for (j = m_iStrip_offset; j < am->iVectorSize; j++) {
            xmu = (*(vector))[j] - pdf->fMean[j];
            sx += xmu * xmu * pdf->fVar[j];
        }

        /// half of it according formula.
        sx *= -0.5;

        /// we are working with the logarithm values always as the original values are getting really small,
        /// and the precision of the computer is not sufficient and will round them to zero
        sx += log(pdf->fWeight);

        /// we are computing the scores for each PDF function for the same state and instead of summing the probabilities
        /// we take the maximum one.
        if (sx > score) {
            score = sx;
        }
    }

    //register computed score
    scores[Index] = score;

    //return computed score
    return score;
}
