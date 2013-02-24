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
#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <utils/XmlReader.h>
#include <utils/CryptoKey.h>

#define RAW2_ENCTAG_SIZE  8192
#define RAW2_COMTAG_SIZE   512
#define RAW2_HASHTAG_SIZE  512

#define ntohll(x) (((int64_t)(ntohl((int32_t)((x << 32) >> 32))) << 32) | (uint32_t)ntohl(((int32_t)(x >> 32)))) 
#define htonll(x) ntohll(x)

namespace sai
{
namespace net2
{

class Raw
{
public:
  uint8_t  encAlgo;
  uint16_t encTagSize;
  uint8_t  comAlgo;
  uint16_t comTagSize;
  uint8_t  hashAlgo;
  uint16_t hashTagSize;
  uint32_t protSize;
  uint32_t xmlSize; 
  uint32_t binSize; 
  char *   encTag;
  char *   comTag;
  char *   hashTag;
  char *   protData;
  char *   xmlData;
  char *   binData;

public:
  Raw();
  virtual ~Raw();
  virtual void print();
};

class ProtMessage;
class Endpoint;
class DataDescriptor
{
public:
  uint8_t                 version;
  Raw *                   raw;
  ProtMessage *           protMessage;
  Endpoint *              endpoint;

public:
  DataDescriptor();
  ~DataDescriptor();

  void print();
  static void HashSAI64(char * msg, uint32_t size, uint64_t& value);
};

typedef enum 
{
  ENC_ALGO_NONE = 0,
  ENC_ALGO_AES256 = sai::utils::AES256,
  ENC_ALGO_ECC521 = sai::utils::ECC521,
  ENC_ALGO_ECC571 = sai::utils::ECC571
}EncryptionAlgo;

typedef enum
{
  COMP_ALGO_NONE = 0
}CompressionAlgo;

typedef enum
{
  HASH_ALGO_NONE   = 0,
  HASH_ALGO_SHA256 = 1,
  HASH_ALGO_SAI64  = 2
}HashingAlgo;

class ProtKeyValue
{
public:
  std::string* key;
  std::string* value;

public:
  ProtKeyValue();
  ~ProtKeyValue();
  void set(std::string k, std::string v) { key->assign(k); value->assign(v); }
};

typedef std::vector<ProtKeyValue*>           ProtList;
typedef std::vector<ProtKeyValue*>::iterator ProtListIterator;

class ProtMessage
{
friend class DataDispatcher;
private:
  static boost::uuids::uuid _uuid;
  ProtList                  _list;

public:
  void fromString(std::string xml);
  void get(const char * name, std::string& value);
  void set(std::string name, std::string  value);

public:
  ProtMessage();
  ~ProtMessage();

  void     setId(uint32_t id);
  uint32_t getId();
  void     toString(std::string& ret, uint32_t& size);
  void     getNodeUUID(std::string&);
  void     setDestination(std::string&);
  void     getDestination(std::string&);

  static void getUUID(std::string&);
};

}}

#endif
