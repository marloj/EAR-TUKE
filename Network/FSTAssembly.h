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
 *  Assembling finite state transducer network. This is part of the conversion tool
 * from HTK acoustic model format to native binary one.
 */

#ifndef __EAR_FSTASSEMBLY_H_
#define __EAR_FSTASSEMBLY_H_

#include "HTKAcousticModel.h"
#include "Dictionary.h"
#include "../Data/Data.h"

#include <map>

using namespace std;

namespace Ear
{
  /**
  *
  */
	class StateManager
	{
	public:
		StateManager();
		~StateManager();

	public:
		unsigned int &operator[](unsigned int _i);
		unsigned int ready(unsigned int _start, unsigned int _last, unsigned int _number);
		unsigned int getNewNumber();

	private:
		void size(unsigned int _size);

	private:
		unsigned int *StateArray;
		unsigned int m_size;
		unsigned int m_count;
	};

  /**
  * Transforming the acoustic model and dictionary to the finite state transducer network (FST).
  * The network can be output as native openFST format with index files or written in binary
  * format with all GMM specifications. For the proper work of the detection system the binary
  * format is mandatory.
  * FST is comprised from states and transitions between them. The transitions are created from the global
  * dictionary file and from acoustic model transition matrixes between individual states of event's models
  * Thus, the transition can hold probability that is result from the event's probability and transition probability
  * in the acoutic model.
  */
	class CFSTAssembly
	{
	public:
		CFSTAssembly();
		~CFSTAssembly();

	public:
    /// Assembles the FST from acoustic model in HTK format and created dictionary
    /// @param [in] _model the model that was read in HTK format
    /// @param [in] _dict the created dictionary of events to detect
    /// @return success state of the function
		int assembly(CHTKAcousticModel *_model, CDictionary *_dict);
    /// Writing native binary format of the FST with acoustic model probability function definitions
    /// along with the index transforming inner number representations to the actual event names
    /// @param [in] _szOut path of the output binary file
    /// @param [in] _szOutIndex path to the output index file
    /// @return success state of the function
		int writeBin(const char *_szOut, const char *_szOutIndex);
    /// Writing only the FST network mostly for debuging purposes in openFST format.
    /// Output can be later used for example for drawing out the network
    /// @param [in] _szFstName path to the text output FST file
    /// @param [in] _iSymName path to the index file for input symbols
    /// @param [in] _oSymName path to the index file for output symbols (the events names)
    /// @return success state of the function
		int writeFST(const char *_szFstName, const char *_iSymName, const char *_oSymName);

	private:
    /// Function for writing the FST file
    /// @param [in] _szFileName path to the output file
    /// @return success of the function
		int writeFSTFile(const char *_szFileName);
    /// Function for writing the output index symbols file
    /// @param [in] _szFileName path to the output file
    /// @return success of the function
		int writeOSymFile(const char *_szFileName);
    /// Function for writing the input index symbols file
    /// @param [in] _szFileName path to the output file
    /// @return success of the function
		int writeISymFile(const char *_szFileName);
    /// Creating the FST using internal state manager, HTK format acoustic model and dictionary
    /// @param [in, out] _states
    /// @param [in] _hmm read acoustic model
    /// @param [in] _word read dictionary linked list
		void createFST(StateManager &_states, HTK_Data *_hmm, DictItem *_word);
    /// Creating/adding new connection between state of output FST
    /// @param [in] _s start state number
    /// @param [in] _e end state number
    /// @param [in] _i input symbol index
    /// @param [in] _o output symbol index
    /// @param [in] _w the transition probability
		void connect(unsigned int _s, unsigned int _e, unsigned int _i, unsigned int _o, float _w);

	private:
		multimap<unsigned int, EAR_FST_Trn*> m_fst; ///< mapping the transition to the start state numbers
		CHTKAcousticModel *m_model; ///< remembering the input HTK format acoustic model
		CDictionary *m_dict; ///< remembering the dictionary
	};


}

#endif
