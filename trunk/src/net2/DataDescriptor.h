//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#ifndef __SAI_NET2_DATADESCRIPTOR__
#define __SAI_NET2_DATADESCRIPTOR__

#include <stdint.h>
#include <utils/XmlReader.h>

namespace sai
{
namespace net2
{

class Raw2
{
public:
  uint8_t  encAlgo;
  uint8_t  encTagSize;
  char     encTag[256];
  uint8_t  comAlgo;
  uint8_t  comTagSize;
  uint8_t  comTag[256];
  uint8_t  xmlAndBinaryFlag;
  uint32_t xmlSize; 
  uint32_t binSize; 
};

typedef union
{
  Raw2 r2;
}Raw;

class Xml2
{
public:
  char * data;
};

typedef union
{
  Xml2 x2;
}Xml;

class Binary2
{
public:
  typedef struct { uint32_t offset; uint32_t size; } Pair;

public:
  uint16_t num;
  Pair *   map;  
  char *   data;
};

typedef union
{
  Binary2 b2;
}Binary;

class DataDescriptor
{
public:
  uint8_t  version;
  Raw      raw;
  Xml      xml;
  Binary   binary;

  sai::utils::XmlReader * xmlReader;
};

}}

#endif
