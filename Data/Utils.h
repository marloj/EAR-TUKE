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
 * This file contains function used mainly by parsing process of the HTK files.
 */

#ifndef __EAR_UTILS_H_
#define __EAR_UTILS_H_

namespace Ear
{
  /// Allocate new array and copy string
  /// @param [in] _c string to copy
  /// @return new memory pointer with copied string
	char *cloneString(char *_c);
  /// Parse used to initiate <i>strtok</i> function with space tokenizer character and returning first token.
  /// This function is deprecated and not used in this moment
  /// @param [in] _s input string to tokenize
  /// @return first token to return
	char *parse(char *_s);
  /// Parse next token. This function is no longer used and is deprecated.
  /// @return next parsed token
  char *parseNext();
  /// Allocate and copy vector of floats into new memory.
  /// @param [in] _f float array to copy
  /// @param [in] _x size of the array to copy
  /// @return copied new array
	float *cloneVector(float *_f, unsigned int _x);
}

#endif
