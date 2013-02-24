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
#include <Objbase.h>
#else
#include <netinet/in.h>
#endif

#include <cstring>
#include <sstream>
#include <cryptlib.h>
#include <sha.h>
#include <utils/CryptoKey.h>
#include <utils/CryptoFactory.h>
#include <net2/ProtId.h>
#include <net2/KeyManager.h>
#include "net2/RawEncoder.h"

#define NET_BUFFER_SIZE 81920

using namespace sai::net2;
using namespace sai::utils;

RawEncoder::RawEncoder() :
  _buffer(0)
{
#ifdef _WIN32
  _buffer = (char*)::CoTaskMemAlloc(NET_BUFFER_SIZE);
#else
  _buffer = new char[NET_BUFFER_SIZE]; // FIXME : Enhance this one to buffer pool
#endif
}

RawEncoder::~RawEncoder()
{
#ifdef _WIN32
  ::CoTaskMemFree(_buffer);
#else
  delete [] _buffer;
#endif
}

#define WRITE(data) memcpy(ptr, &data, sizeof(data)); ptr += sizeof(data)
#define WRITEN(data,size) memcpy(ptr, data, size); ptr += size

char * 
RawEncoder::make(DataDescriptor& desc, uint32_t& size)
{
  desc.version = 3; // Always

  uint32_t magic = 0x000cdb1c;
  magic = htonl(magic);

  char * ptr =_buffer;
  WRITE(magic);
  
  uint16_t version = htons(desc.version);
  WRITE(version);

  char * size3Ptr = ptr;
  uint32_t size3 = 0;
  WRITE(size3);

  uint16_t seqNo = 0;
  WRITE(seqNo);

  switch(desc.raw->hashAlgo)
  {
    case HASH_ALGO_SHA256:
      {
        CryptoPP::SHA256 sha256;
        desc.raw->hashTagSize = sha256.DigestSize();
      }
      break;
    case HASH_ALGO_SAI64:
      {
        desc.raw->hashTagSize = sizeof(uint64_t);
      }
      break;
  }

  switch(desc.raw->comAlgo)
  {
    case COMP_ALGO_NONE:
      break;
    default:
      // TODO : Compress
      break;
  }

  std::string* eProtDataString = new std::string();
  std::string* eXmlDataString = new std::string();
  std::string* eBinDataString = new std::string();

  std::string* protData = new std::string();
  desc.protMessage->toString(*protData, desc.raw->protSize);
  desc.raw->protData = (char*)protData->data();

  switch(desc.raw->encAlgo)
  {
    case ENC_ALGO_NONE:
      {
        eProtDataString->assign(desc.raw->protData, desc.raw->protSize);
        eXmlDataString->assign(desc.raw->xmlData, desc.raw->xmlSize);
        eBinDataString->assign(desc.raw->binData, desc.raw->binSize);

        if (desc.protMessage->getId() == PROTID_AUTH_PUBKEY_REQUEST)
        {
          AsymmetricKey* key = KeyManager::GetInstance()->getAsymmetricKey();
          memset(desc.raw->encTag, 0, RAW2_ENCTAG_SIZE);
          std::string pub;
          key->getPublicKeyString(pub);
          memcpy(desc.raw->encTag, pub.data(), pub.size());
          desc.raw->encTagSize = pub.size();
          pub.clear();
        }
      }
      break;
    case ENC_ALGO_AES256:
      {
        CryptoKeyFactory keyFactory;
        CryptoFactory cryptFactory;

        SymmetricKey * key = KeyManager::GetInstance()->getSymmetricKey();
        SymmetricCrypto * crypt = static_cast<SymmetricCrypto*>(cryptFactory.create(AES256));

        memset(desc.raw->encTag, 0, RAW2_ENCTAG_SIZE);
        key->randomIV();
        std::string iv;
        key->getIVString(iv);
        memcpy(desc.raw->encTag, iv.data(), iv.size());
        desc.raw->encTagSize = iv.size();
        iv.clear();

        std::string* data = new std::string();

        if (desc.raw->protSize > 0)
        {
          data->assign(desc.raw->protData, desc.raw->protSize);
          crypt->encrypt(*key, *data, *eProtDataString);
        }

        if (desc.raw->xmlSize > 0)
        {
          data->assign(desc.raw->xmlData, desc.raw->xmlSize);
          crypt->encrypt(*key, *data, *eXmlDataString);
        }

        if (desc.raw->binSize > 0)
        {
          data->assign(desc.raw->binData, desc.raw->binSize);
          crypt->encrypt(*key, *data, *eBinDataString);
        }

        delete data;
        delete crypt;
      }
      break;
    case ENC_ALGO_ECC521:
      {
        CryptoKeyFactory keyFactory;
        CryptoFactory cryptFactory;

        AsymmetricKey* key  = KeyManager::GetInstance()->getAsymmetricKey();
        AsymmetricKey* okey = KeyManager::GetInstance()->getOthersPublicKey();
        AsymmetricCrypto* crypt = static_cast<AsymmetricCrypto*>(cryptFactory.create(ECC521));
        memset(desc.raw->encTag, 0, RAW2_ENCTAG_SIZE);
        std::string pub;
        key->getPublicKeyString(pub);
        assert(pub.size() > 0 && pub.size() <= RAW2_ENCTAG_SIZE);
        memcpy(desc.raw->encTag, pub.data(), pub.size());
        desc.raw->encTagSize = pub.size();
        pub.clear();

        std::string* data = new std::string();

        if (desc.raw->protSize > 0)
        {
          data->assign(desc.raw->protData, desc.raw->protSize);
          crypt->encrypt(*okey, *data, *eProtDataString);
        }

        if (desc.raw->xmlSize > 0)
        {
          data->assign(desc.raw->xmlData, desc.raw->xmlSize);
          crypt->encrypt(*okey, *data, *eXmlDataString);
        }

        if (desc.raw->binSize > 0)
        {
          data->assign(desc.raw->binData, desc.raw->binSize);
          crypt->encrypt(*okey, *data, *eBinDataString);
        }

        delete data;
        delete crypt;
      }
      break;
    case ENC_ALGO_ECC571:
      {
      }
      break;
    default:
      break;
  }

  uint16_t encTagSize = htons(desc.raw->encTagSize);
  WRITE(desc.raw->encAlgo);
  WRITE(encTagSize);

  uint16_t comTagSize = htons(desc.raw->comTagSize);
  WRITE(desc.raw->comAlgo);
  WRITE(comTagSize);

  uint16_t hashTagSize = htons(desc.raw->hashTagSize);
  WRITE(desc.raw->hashAlgo);
  WRITE(hashTagSize);

  uint32_t eProtSize, eXmlSize, eBinSize;
  char *eProtData, *eXmlData, *eBinData;

  eProtSize = eProtDataString->size();
  eXmlSize  = eXmlDataString->size();
  eBinSize  = eBinDataString->size();
  desc.raw->protSize = eProtSize;
  desc.raw->xmlSize  = eXmlSize;
  desc.raw->binSize  = eBinSize;
  eProtData = (char*)eProtDataString->data();
  eXmlData  = (char*)eXmlDataString->data();
  eBinData  = (char*)eBinDataString->data();

  eProtSize = htonl(eProtSize);
  eXmlSize  = htonl(eXmlSize);
  eBinSize  = htonl(eBinSize);
  WRITE(eProtSize);
  WRITE(eXmlSize);
  WRITE(eBinSize);

  if (desc.raw->encTagSize > 0)  WRITEN(desc.raw->encTag,  desc.raw->encTagSize);
  if (desc.raw->comTagSize > 0)  WRITEN(desc.raw->comTag,  desc.raw->comTagSize);
  char * hashStartPtr = ptr;
  if (desc.raw->hashTagSize > 0) WRITEN(desc.raw->hashTag, desc.raw->hashTagSize);

  char * hashDataStartPtr = ptr;

  if (desc.raw->protSize > 0) WRITEN(eProtData, desc.raw->protSize);
  if (desc.raw->xmlSize > 0)  WRITEN(eXmlData,  desc.raw->xmlSize);
  if (desc.raw->binSize > 0)  WRITEN(eBinData,  desc.raw->binSize);

  switch(desc.raw->hashAlgo)
  {
    case HASH_ALGO_SHA256:
      {
        CryptoPP::SHA256 sha256;
        CryptoPP::SecByteBlock digest256(sha256.DigestSize());
        sha256.Update((const byte*)hashDataStartPtr, desc.raw->protSize + desc.raw->xmlSize + desc.raw->binSize);
        sha256.Final(digest256);
        uint8_t * ptr1, * ptr2;
        ptr1 = (uint8_t*)desc.raw->hashTag;
        ptr2 = (uint8_t*)hashStartPtr;
        for (uint32_t i = 0; i < sha256.DigestSize(); i += 1, ptr1++, ptr2++)
        {
          *ptr1 = digest256[i]; 
          *ptr2 = digest256[i]; 
        }
      }
      break;
    case HASH_ALGO_SAI64:
      {
        uint64_t v;
        DataDescriptor::HashSAI64(hashDataStartPtr, desc.raw->protSize + desc.raw->xmlSize + desc.raw->binSize, v);

        v = htonll(v);
        memcpy(desc.raw->hashTag, &v, sizeof(v));
        memcpy(hashStartPtr, &v, sizeof(v));
      }
      break;
    default:
      break;
  }

  delete protData;
  delete eBinDataString;
  delete eXmlDataString;
  delete eProtDataString;

  _size = size = (ptr - _buffer);

  size3 = size;
  size3 = htonl(size3);
  memcpy(size3Ptr, &size3, sizeof(size3)); 

  return _buffer;
}

