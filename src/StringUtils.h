/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * OpenDMX driver from OLA. 
 * adapted for node.js with fixed darwin support
 * Copyright (C) 2010 Simon Newton              
 * Copyright (C) 2013 Nicklas Marelius
 */
 
#ifndef StringUtils_H_
#define StringUtils_H_

#include <stdint.h>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;

void StringSplit(const string &input,
                 vector<string> &tokens,
                 const string &delimiters=" ");
void StringTrim(string *input);
void ShortenString(string *input);
bool StringEndsWith(const string &s, const string &ending);
string IntToString(int i);
string IntToString(unsigned int i);
void Escape(string *original);
string EscapeString(const string &original);
bool StringToInt(const string &value,
                 unsigned int *output,
                 bool strict = false);
bool StringToInt(const string &value, uint16_t *output, bool strict = false);
bool StringToInt(const string &value, uint8_t *output, bool strict = false);
bool StringToInt(const string &value, int *output, bool strict = false);
bool StringToInt(const string &value, int16_t *output, bool strict = false);
bool StringToInt(const string &value, int8_t *output, bool strict = false);
bool HexStringToInt(const string &value, uint8_t *output);
bool HexStringToInt(const string &value, uint16_t *output);
bool HexStringToInt(const string &value, uint32_t *output);
bool HexStringToInt(const string &value, int8_t *output);
bool HexStringToInt(const string &value, int16_t *output);
bool HexStringToInt(const string &value, int32_t *output);
void ToLower(string *s);
void ToUpper(string *s);
void CapitalizeLabel(string *s);
void CustomCapitalizeLabel(string *s);
void FormatData(std::ostream *out,
                const uint8_t *data,
                unsigned int length,
                unsigned int indent = 0,
                unsigned int byte_per_line = 8);

/**
 * Convert a hex string, prefixed with 0x to an int type.
 */
template <typename int_type>
bool PrefixedHexStringToInt(const string &input, int_type *output) {
  if (input.find("0x") != 0)
    return false;
  string modified_input = input.substr(2);
  return HexStringToInt(modified_input, output);
}

/**
 * Join a vector of type T.
 * T can be any type for which the << operator is defined
 */
template<typename T>
string StringJoin(const string &delim, const T &input) {
  std::ostringstream str;
  typename T::const_iterator iter = input.begin();
  while (iter != input.end()) {
    str << *iter++;
    if (iter != input.end())
      str << delim;
  }
  return str.str();
}

#endif
