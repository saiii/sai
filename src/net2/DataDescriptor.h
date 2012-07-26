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
#include <map>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
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
  uint8_t  comAlgo;
  uint8_t  comTagSize;
  uint8_t  hashAlgo;
  uint8_t  hashTagSize;
  uint32_t protSize;
  uint32_t xmlSize; 
  uint32_t binSize; 
  uint8_t  encTag[256];
  uint8_t  comTag[256];
  uint8_t  hashTag[256];
  char *   protData;
  char *   xmlData;
  char *   binData;

public:
  void print();
};

typedef union
{
  Raw2 r2;
}Raw;

class ProtMessage;
class DataDescriptor
{
public:
  uint8_t                 version;
  Raw                     raw;
  sai::utils::XmlReader * xmlProtReader;
  sai::utils::XmlReader * xmlDataReader;
  ProtMessage *           protMessage;

public:
  DataDescriptor();
  ~DataDescriptor();

  void print();
};

typedef enum 
{
  ENC_ALGO_NONE = 0
}EncryptionAlgo;

typedef enum
{
  COMP_ALGO_NONE = 0
}CompressionAlgo;

typedef enum
{
  HASH_ALGO_NONE   = 0,
  HASH_ALGO_SHA256 = 1
}HashingAlgo;

typedef std::map<std::string, std::string>           ProtMap;
typedef std::map<std::string, std::string>::iterator ProtMapIterator;

class ProtMessage
{
private:
  static boost::uuids::uuid _uuid;
  ProtMap                   _map;
  std::string               _message;

public:
  ProtMessage();
  ~ProtMessage();

  void setId(uint32_t id);
  void toString(std::string& ret);
  uint32_t size();

  static std::string getUUID(std::string&);
};

}}

#endif
