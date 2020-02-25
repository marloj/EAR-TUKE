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
 * Reading HTK format of the acoustic model. This is part of tool for conversion HTK formatted acoustic model
 * to native binary one. This file contains parsing the text files and structure definitions. All information
 * of the acoustic model is saved in linked lists.
 */

#ifndef __EAR_HTK_ACOUSTICMODEL_H_
#define __EAR_HTK_ACOUSTICMODEL_H_

#include <stdio.h>
#include <map>
#include <string.h>

#include "../Data/Data.h"
#include "../Data/FileIO.h"

using namespace std;
namespace Ear {
    /// Transitions/transposition matrix definition (linked list)

    typedef struct HTK_Trans_ {
        char *name; ///< name of the matrix if specified as macro in the file, NULl otherwise
        unsigned int id; ///< incremental ids for the matrices in future native array
        float **p; ///< transposition matrix
        int size; ///< size of the matrix (this is always square matrix as it describes transitions between all the states in acoustic model)
        HTK_Trans_ *next; ///< next matrix
    } HTK_Trans;

    /// Probability density function specification (linked list)

    typedef struct HTK_Pdf_ {
        char *name; ///< name of the function (if specified as macro, NULL otherwise)
        unsigned int id; ///< incremental ids for functions in future native array
        float *var; ///< variance of the GMM (multidimensional)
        float *mean; ///< mean value of the GMM (multidimensional)
        float weight; ///< weight of the function
        float gconst; ///< precomputed value of the function
        HTK_Pdf_ *next; ///< next function (linked list)
    } HTK_Pdf;

    /// States definitions (linked list)

    typedef struct HTK_State_ {
        char *name; ///< name of the state if defined as macro, NULL otherwise
        unsigned int id; ///< incremental id for the states in future native array
        unsigned int ipdfs; ///< number of states functions for current state
        HTK_Pdf **pdfs; ///< array of the functions
        HTK_State_ *next; ///< next state definition (linked list)
    } HTK_State;

    /// One event model definition

    typedef struct HTK_Data_ {
        unsigned int id; ///< incremental id for model in future native array
        short is; ///< Number of states for this model
        char *name; ///< Name of the event's name
        HTK_Trans *tr; ///< transitions/transposition matrices
        HTK_State **s; ///< states
        HTK_Data_ *next; ///< next model (linked list)
    } HTK_Data;

    /// Top level of HTK format acoustic model definition

    typedef struct HTK_Info_ {
        short iVectorSize; ///< global vector size of features vectors and PDF functions
        char szType[100]; ///< type of the acoustic model, for exmp. MFCC_D_A_Z
        unsigned int idata; ///< number of models
        unsigned int iStates; ///< total number of states
        unsigned int iPdfs; ///< total number of PDF functions
        unsigned int iTrs; ///< total number of transition matrices
        HTK_Data *data; ///< models
        HTK_State *s; ///< all states
        HTK_Pdf *pdf; ///< all PDF functions
        HTK_Trans *trs; ///< all transition matrices
    } HTK_Info;

    /**
     * Reading the acoustic model in HTK format from text file. This class also contains methods to convert
     * the model to the native one.
     */
    class CHTKAcousticModel {
    private:
        /// Operator for hash map orderings as we will use char* as items

        struct cmp_str {

            bool operator()(char const *a, char const *b) const {
                return strcmp(a, b) < 0;
            }
        };

    private:
        /// Top level structure to hold model parameters
        HTK_Info m_HTK_info;

        /// For easy searching in data in case they were defined by macro not inline
        map<char*, HTK_Trans*, cmp_str> mapTrs; ///< for matrices (macro ~t)
        map<char*, HTK_State*, cmp_str> mapStates; ///< for states (macro ~s)
        map<char*, HTK_Data*, cmp_str> mapData; ///< for models (macro ~h)

        FileIO m_file; ///< HTK model file

    public:
        CHTKAcousticModel();
        ~CHTKAcousticModel();
        /// Load the model from file in HTK format
        /// @param [in] _szFileName file to open and parse
        /// @return success of the parsing
        int loadModelFrom(char *_szFileName);
        /// Search for named model in the acoustic model
        /// @param [in] _name name of the model
        /// @return pointer to the model's structure
        HTK_Data *searchForData(char *_name);
        /// Get the whole acoustic model
        /// @return pointer to the top level structure of the acoustic model
        HTK_Info *getHMMInfo();
        /// Get acoustic model converted to the native format structure. This function can be used for reading
        /// HTK format acoustic model into native format.
        /// @return native format of the acoustic model
        EAR_AM_Info *getAcousticModel();

    private:
        /// Search for transposition matrix if defined by macro
        /// @param [in] _name the name of the matrix
        /// @return pointer to the matrix definition structure
        HTK_Trans *searchForTransp(char *_name);
        /// Search for states that were defined as macros
        /// @param [in] _name name of the state
        /// @return pointer to the state definition
        HTK_State *searchForStates(char *_name);
        /// Helper function for reading in 2D matrix from text file
        /// @param [in] _x x dimension of the matrix to read
        /// @param [in] _y y dimension of the matrix to read
        /// @param [in] _log whether to compute logarithm of the input values
        /// @return array of the values read
        float **read2Matrix(unsigned int _x, unsigned int _y, unsigned int _log);
        /// Helper function for reading the input vector
        /// @param [in] _x the size of the vector
        /// @return array of the values read
        float *readVector(unsigned int _x);
        /// Invert each value in the vector by dividing (1/x). This should be matrix inversion, but as we are using
        /// only diagonal matrices for variance of the PDFs (thus vector representation), the inversion is easier.
        /// @param [in] the vector
        /// @param [in] _size size of the vector
        void invert(float *_f, unsigned int _size);
        /// Consume the header of the HTK format acoustic model. Parses only the header of the model.
        /// @return success of the function
        int readHMMHeader();
        /// Consume the rest of the acoustic model and parse the values. This is top level
        /// function for parsing the acoustic model
        /// @return success of the function
        int readHMMInfo();
        /// Consume the transposition/transition matrix from acoustic model. The function triggers <i>add</i> function
        /// for adding the matrix into top level structure of the acoustic model.
        /// @param [in] _name set name of the matrix if available
        /// @return the newly created matrix definition
        HTK_Trans *saveTransp(char *_name);
        /// Consume the state definition from acoustic model. The function triggers the <i>add</i> function
        /// for adding the state into top level structure of the acoustic model. Also triggers another functions
        /// to consume PDF functions <i>savePdf</i>
        /// @param [in] _name set name of the state if available
        /// @return the newly created state definition
        HTK_State *saveState(char *_name);
        /// Consume the model definition from acoustic model. The function triggers the <i>add</i> function
        /// for adding the model into top level structure of the acoustic model. Also triggers another functions
        /// to consume states <i>saveState</i>
        /// @param [in] _name set name of the model if available
        /// @return the newly created model definition
        HTK_Data *saveModel(char *_name);
        /// Consume the PDF function definition from acoustic model. The function triggers the <i>add</i> function
        /// for adding the state into top level structure of the acoustic model.
        /// @param [in] _name set name of the PDF if available
        /// @return the newly created PDF definition
        HTK_Pdf *savePdf(char *_name);
        /// Consume macro definition of transposition matrix
        /// @return success of the function
        int macroTransp();
        /// Consume macro definition of state
        /// @return success of the function
        int macroState();
        /// Consume macro definiton of model
        /// @return success of the function
        int macroModel();
        /// Helper function for adding transition matrix into top level structure (linked lists) and increasing appropriate values
        /// @param [in] _item matrix to add
        void add(HTK_Trans *_item);
        /// Helper function for adding state into top level structure (linked lists) and increasing appropriate values
        /// @param [in] _item state to add
        void add(HTK_State *_item);
        /// Helper function for adding model into top level structure (linked lists) and increasing appropriate values
        /// @param [in] _item model to add
        void add(HTK_Data *_item);
        /// Helper function for adding PDF into top level structure (linked lists) and increasing appropriate values
        /// @param [in] _item PDF to add
        void add(HTK_Pdf *_item);
    };
}

#endif
