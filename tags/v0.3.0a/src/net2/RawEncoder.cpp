//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <cstring>
#include <sstream>
#include <cryptlib.h>
#include <sha.h>
#include "net2/RawEncoder.h"

#define NET_BUFFER_SIZE 8192

using namespace sai::net2;

RawEncoder::RawEncoder() :
  _buffer(0)
{
  _buffer = new char[NET_BUFFER_SIZE]; // FIXME : Too large??
}

RawEncoder::~RawEncoder()
{
  delete [] _buffer;
}

#define WRITE(data) memcpy(ptr, &data, sizeof(data)); ptr += sizeof(data)
#define WRITEN(data,size) memcpy(ptr, data, size); ptr += size

char * 
RawEncoder::make(DataDescriptor& desc, uint32_t& size)
{
  desc.version = 2; // Always

  uint32_t magic = 0x000cdb1c;
  magic = htonl(magic);

  char * ptr =_buffer;
  WRITE(magic);
  
  uint16_t version = htons(desc.version);
  WRITE(version);

  switch(desc.raw.r2.hashAlgo)
  {
    case HASH_ALGO_SHA256:
      {
        CryptoPP::SHA256 sha256;
        desc.raw.r2.hashTagSize = sha256.DigestSize();
      }
      break;
  }

  WRITE(desc.raw.r2.encAlgo);
  WRITE(desc.raw.r2.encTagSize);
  WRITE(desc.raw.r2.comAlgo);
  WRITE(desc.raw.r2.comTagSize);
  WRITE(desc.raw.r2.hashAlgo);
  WRITE(desc.raw.r2.hashTagSize);

  switch(desc.raw.r2.comAlgo)
  {
    case COMP_ALGO_NONE:
      break;
    default:
      // TODO : Compress
      break;
  }

  switch(desc.raw.r2.encAlgo)
  {
    case ENC_ALGO_NONE:
      break;
    default:
      // TODO : Encrypt
      break;
  }

  uint32_t eProtSize, eXmlSize, eBinSize;
  char *eProtData, *eXmlData, *eBinData;

  std::string protData;
  desc.protMessage->toString(protData);
  desc.raw.r2.protSize = desc.protMessage->size();
  desc.raw.r2.protData = (char*)protData.data();

  eProtSize = desc.raw.r2.protSize;
  eXmlSize  = desc.raw.r2.xmlSize;
  eBinSize  = desc.raw.r2.binSize;
  eProtData = desc.raw.r2.protData;
  eXmlData  = desc.raw.r2.xmlData;
  eBinData  = desc.raw.r2.binData;

  eProtSize = htonl(eProtSize);
  eXmlSize  = htonl(eXmlSize);
  eBinSize  = htonl(eBinSize);
  WRITE(eProtSize);
  WRITE(eXmlSize);
  WRITE(eBinSize);

  if (desc.raw.r2.encTagSize > 0)  WRITEN(desc.raw.r2.encTag,  desc.raw.r2.encTagSize);
  if (desc.raw.r2.comTagSize > 0)  WRITEN(desc.raw.r2.comTag,  desc.raw.r2.comTagSize);
  char * hashStartPtr = ptr;
  if (desc.raw.r2.hashTagSize > 0) WRITEN(desc.raw.r2.hashTag, desc.raw.r2.hashTagSize);

  char * hashDataStartPtr = ptr;

  if (desc.raw.r2.protSize > 0) WRITEN(eProtData, desc.raw.r2.protSize);
  if (desc.raw.r2.xmlSize > 0)  WRITEN(eXmlData,  desc.raw.r2.xmlSize);
  if (desc.raw.r2.binSize > 0)  WRITEN(eBinData,  desc.raw.r2.binSize);

  _size = size = (ptr - _buffer);

  switch(desc.raw.r2.hashAlgo)
  {
    case HASH_ALGO_SHA256:
      {
        CryptoPP::SHA256 sha256;
        CryptoPP::SecByteBlock digest256(sha256.DigestSize());
        sha256.Update((const byte*)hashDataStartPtr, desc.raw.r2.protSize + desc.raw.r2.xmlSize + desc.raw.r2.binSize);
        sha256.Final(digest256);
        uint8_t * ptr1, * ptr2;
        ptr1 = (uint8_t*)desc.raw.r2.hashTag;
        ptr2 = (uint8_t*)hashStartPtr;
        for (uint32_t i = 0; i < sha256.DigestSize(); i += 1, ptr1++, ptr2++)
        {
          *ptr1 = digest256[i]; 
          *ptr2 = digest256[i]; 
        }
      }
      break;
    default:
      break;
  }

  return _buffer;
}

