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
* This file holds definition for reading binary format of the resources needed by
* the decoding process. Those resources are:
* 1. Acoustic model
* 2. Finite State Tranducer (search network)
* 3. Dictionary of the events names to be detected.
*/

#ifndef __EAR_DATAREADER_H_
#define __EAR_DATAREADER_H_

#include "Data.h"
#include <map>

namespace Ear
{
  /**
  * Holder for the read acoustic model and finite state transducer (search network) and dictionary
  * Contains function for reading the binary representation of those resources as this
  * software is using its own binary format. The two resources are saved in the same
  * binary file, while the dictionary mapping output number to event names is saved in another "index" text file.
  */
	class CDataHolder
	{
	public:
		CDataHolder();
		~CDataHolder();

  public:
    /** Function for loading the resources from file. The binary file has following format
    * 1. Vector size - unsigned 2 bytes
    * 2. Number of states - unsigned 4 bytes
    * 3. Number of PDFs - unsigned 4 bytes
    * 4. Number of PDFs on state - unsigned 4 bytes
    * 5. Array of indexes of PDFs - Number of PDFs * unsigned 4 bytes
    * 6. PDF (variance, mean, gconst, weight) - Number of PDFs * 4 * 4 bytes (float)
    * 7. FST size - number of transitions in search network - unsigned 5 bytes
    * 8. FST (start, end, in symbol, out symbol, weight) - FST size * ( 4 * unsigned 4 bytes, 4 bytes (float))
    * @param [in] _szFileName name of the file to read
    * @param [in] _szIndexName name of the index file to read (the dictionary)
    * @return status of the loading EAR_SUCCESS or EAR_FAIL
    */
	  unsigned int load(const char *_szFileName, const char *_szIndexName);
    /// Function for getting acoustic model from the loaded resources
    /// @return pointer to the structure of acoustic model
	  EAR_AM_Info *getAcousticData();
    /// Function for getting the search network part of the loaded data
    /// @return pointer to structure of finite state transducer
	  EAR_FST_Net *getFSTData();
    /// Function for getting dictionary from loaded index file
    /// @return pointer to structure of dictionary
	  EAR_Dict *getDict();

	private:
		EAR_AM_Info am;   ///< read acoustic model
		EAR_FST_Net fst;  ///< read finite state transducer
		EAR_Dict mapWords;///< read dictionary

    /// Originally the network consists from states that are numbered, so transition is defined
    /// by two states, one starting point and one ending point. We are remembering the transitions
    /// in one continuous array as a list. For better performance we will re-map the end state
    /// numbers to indexes of the array. Each index is representing the beginning of the transition list
    /// for the desired state by using this temporary hash map.
    std::map<unsigned int, unsigned int> mapStates;
	};
}

#endif
