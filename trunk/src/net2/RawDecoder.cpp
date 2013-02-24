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

#include <cstdio>
#include <string.h>
#include <cryptlib.h>
#include <sha.h>
#include <net/ProtocolDecoder.h>
#include <utils/CryptoKey.h>
#include <utils/CryptoFactory.h>
#include <net2/DataDescriptor.h>
#include <net2/DataDispatcher.h>
#include <net2/RawDecoder.h>
#include <net2/KeyManager.h>

using namespace sai::net2;
using namespace sai::utils;

namespace sai
{
namespace net2
{

class Decoder3
{
private:
#ifdef _WIN32
  CRITICAL_SECTION  _mutex;
#else
  pthread_mutex_t   _mutex;
#endif
  DataDispatcher * disp;

public:
  Decoder3(DataDispatcher * d);
  ~Decoder3();

  void processData(Endpoint* endpoint, const char * data, const uint32_t size);
  void lock();
  void unlock();
};

inline void Decoder3::lock()
{
#ifdef _WIN32
  EnterCriticalSection(&_mutex);
#else
  pthread_mutex_lock(&_mutex);
#endif
}

inline void Decoder3::unlock()
{
#ifdef _WIN32
  LeaveCriticalSection(&_mutex);
#else
  pthread_mutex_unlock(&_mutex);
#endif
}

}}

Decoder3::Decoder3(DataDispatcher * d):
  disp(d)
{
#ifdef _WIN32
  InitializeCriticalSection(&_mutex);
#else
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&_mutex, &attr);
  pthread_mutexattr_destroy(&attr);
#endif
}

Decoder3::~Decoder3()
{
#ifdef _WIN32
  DeleteCriticalSection(&_mutex);
#endif
}

// Magic <2>, Version <1>
// EncAlgo <1>, EncTagSize <1>, EncTag <N>
// ComAlgo <1>, ComTagSize <1>, ComTag <N>
// XML/BINARY Flag <1>
// XMLSIZE <4>
// BINSIZE <4>
// XML <N>
// BIN <N>

#define READ(dest) memcpy(& dest, ptr, sizeof(dest)); ptr += sizeof(dest)
#define READN(dest,size) memcpy(dest, ptr, size); ptr += size

void 
Decoder3::processData(Endpoint * endpoint, const char * data, const uint32_t size)
{
  if (size < 18) 
  {
    return;
  }

  // Magic, version, seqNo and size are already verified so skip it
  char * ptr = (char *) (data + 12);

  DataDescriptor desc;
  desc.endpoint = endpoint;
  desc.version = 3;

  desc.raw->encAlgo = 0;
  READ(desc.raw->encAlgo);
  READ(desc.raw->encTagSize); desc.raw->encTagSize = ntohs(desc.raw->encTagSize);
  READ(desc.raw->comAlgo);
  READ(desc.raw->comTagSize); desc.raw->comTagSize = ntohs(desc.raw->comTagSize);
  READ(desc.raw->hashAlgo);
  READ(desc.raw->hashTagSize);desc.raw->hashTagSize= ntohs(desc.raw->hashTagSize);
  READ(desc.raw->protSize);
  READ(desc.raw->xmlSize);
  READ(desc.raw->binSize);

  if (desc.raw->protSize > 0) desc.raw->protSize = ntohl(desc.raw->protSize);
  if (desc.raw->xmlSize > 0)  desc.raw->xmlSize  = ntohl(desc.raw->xmlSize);
  if (desc.raw->binSize > 0)  desc.raw->binSize  = ntohl(desc.raw->binSize);

  if (desc.raw->encTagSize > 0)  READN(desc.raw->encTag,  desc.raw->encTagSize); 
  if (desc.raw->comTagSize > 0)  READN(desc.raw->comTag,  desc.raw->comTagSize);
  if (desc.raw->hashTagSize > 0) READN(desc.raw->hashTag, desc.raw->hashTagSize);

  switch(desc.raw->hashAlgo)
  {
    case HASH_ALGO_SHA256:
    {
      CryptoPP::SHA256 sha256;
      CryptoPP::SecByteBlock digest256(sha256.DigestSize());
      sha256.Update((const byte*)ptr, desc.raw->protSize + desc.raw->xmlSize + desc.raw->binSize);
      sha256.Final(digest256);

      uint8_t* ptr1 = (uint8_t*)desc.raw->hashTag;
      for(uint32_t i = 0; i < sha256.DigestSize(); i += 1, ptr1++)
      {
        if (*ptr1 != digest256[i]) 
        {
          return;
        }
      }
    }
    break;
    case HASH_ALGO_SAI64:
    {
      uint64_t v;
      DataDescriptor::HashSAI64(ptr, desc.raw->protSize + desc.raw->xmlSize + desc.raw->binSize, v);

      uint64_t m;
      memcpy(&m, desc.raw->hashTag, sizeof(m));
      m = ntohll(m);

      if (v != m) 
      {
        return;
      }
    }
    break;
  }

  char * protDataPtr, * xmlDataPtr, * binDataPtr;
  protDataPtr = xmlDataPtr = binDataPtr = 0;

  if (desc.raw->protSize > 0)
  {
#ifdef _WIN32
    desc.raw->protData = (char*)::CoTaskMemAlloc(desc.raw->protSize);
#else
    desc.raw->protData = new char[desc.raw->protSize];
#endif
    READN(desc.raw->protData, desc.raw->protSize);
    protDataPtr = desc.raw->protData;
  }

  if (desc.raw->xmlSize > 0)
  {
#ifdef _WIN32
    desc.raw->xmlData = (char*)::CoTaskMemAlloc(desc.raw->xmlSize);
#else
    desc.raw->xmlData = new char[desc.raw->xmlSize];
#endif
    READN(desc.raw->xmlData, desc.raw->xmlSize);
    xmlDataPtr = desc.raw->xmlData;
  }

  if (desc.raw->binSize > 0)
  {
#ifdef _WIN32
    desc.raw->binData = (char*)::CoTaskMemAlloc(desc.raw->binSize);
#else
    desc.raw->binData = new char[desc.raw->binSize];
#endif
    READN(desc.raw->binData, desc.raw->binSize);
    binDataPtr = desc.raw->binData;
  }

  std::string* dProtDataString = new std::string();
  std::string* dXmlDataString = new std::string();
  std::string* dBinDataString = new std::string();
  char * dProtDataPtr = 0;
  char * dXmlDataPtr = 0;
  char * dBinDataPtr = 0;
  switch(desc.raw->encAlgo)
  {
    case ENC_ALGO_NONE:
      {
      }
      break;
    case ENC_ALGO_AES256:
      {
        CryptoKeyFactory keyFactory;
        CryptoFactory cryptFactory;

        SymmetricKey * key = KeyManager::GetInstance()->getSymmetricKey();
        SymmetricCrypto * crypt = static_cast<SymmetricCrypto*>(cryptFactory.create(AES256));

        std::string iv; 
        iv.append(desc.raw->encTag, desc.raw->encTagSize);
        key->setIV(iv);
        iv.clear();

        std::string* data = new std::string();

        if (desc.raw->protSize > 0)
        {
          data->assign(desc.raw->protData, desc.raw->protSize);
          crypt->decrypt(*key, *data, *dProtDataString);
#ifdef _WIN32
          dProtDataPtr = (char*)::CoTaskMemAlloc(dProtDataString->size());
#else
          dProtDataPtr = new char[dProtDataString->size()];
#endif
          memcpy(dProtDataPtr, dProtDataString->data(), dProtDataString->size());
          desc.raw->protData = dProtDataPtr;
          desc.raw->protSize = dProtDataString->size();
        }

        if (desc.raw->xmlSize > 0)
        {
          data->assign(desc.raw->xmlData, desc.raw->xmlSize);
          crypt->decrypt(*key, *data, *dXmlDataString);
#ifdef _WIN32
          dXmlDataPtr = (char*)::CoTaskMemAlloc(dXmlDataString->size());
#else
          dXmlDataPtr = new char[dXmlDataString->size()];
#endif
          memcpy(dXmlDataPtr, dXmlDataString->data(), dXmlDataString->size());
          desc.raw->xmlData = dXmlDataPtr;
          desc.raw->xmlSize = dXmlDataString->size();
        }

        if (desc.raw->binSize > 0)
        {
          data->assign(desc.raw->binData, desc.raw->binSize);
          crypt->decrypt(*key, *data, *dBinDataString);
#ifdef _WIN32
          dBinDataPtr = (char*)::CoTaskMemAlloc(dBinDataString->size());
#else
          dBinDataPtr = new char[dBinDataString->size()];
#endif
          memcpy(dBinDataPtr, dBinDataString->data(), dBinDataString->size());
          desc.raw->binData = dBinDataPtr;
          desc.raw->binSize = dBinDataString->size();
        }

        delete data;
        delete crypt;
      }
      break;
    case ENC_ALGO_ECC521:
      {
        std::string o;
        o.assign((char*)desc.raw->encTag, desc.raw->encTagSize);
        KeyManager::GetInstance()->setOthersPublicKey(o);
        o.clear();

        CryptoKeyFactory keyFactory;
        CryptoFactory cryptFactory;

        AsymmetricKey* key = KeyManager::GetInstance()->getAsymmetricKey();
        AsymmetricCrypto * crypt = static_cast<AsymmetricCrypto*>(cryptFactory.create(ECC521));

        std::string* data = new std::string();

        if (desc.raw->protSize > 0)
        {
          data->assign(desc.raw->protData, desc.raw->protSize);
          crypt->decrypt(*key, *data, *dProtDataString);
#ifdef _WIN32
          dProtDataPtr = (char*)::CoTaskMemAlloc(dProtDataString->size());
#else
          dProtDataPtr = new char[dProtDataString->size()];
#endif
          memcpy(dProtDataPtr, dProtDataString->data(), dProtDataString->size());
          desc.raw->protData = dProtDataPtr;
          desc.raw->protSize = dProtDataString->size();
        }

        if (desc.raw->xmlSize > 0)
        {
          data->assign(desc.raw->xmlData, desc.raw->xmlSize);
          crypt->decrypt(*key, *data, *dXmlDataString);
#ifdef _WIN32
          dXmlDataPtr = (char*)::CoTaskMemAlloc(dXmlDataString->size());
#else
          dXmlDataPtr = new char[dXmlDataString->size()];
#endif
          memcpy(dXmlDataPtr, dXmlDataString->data(), dXmlDataString->size());
          desc.raw->xmlData = dXmlDataPtr;
          desc.raw->xmlSize = dXmlDataString->size();
        }

        if (desc.raw->binSize > 0)
        {
          data->assign(desc.raw->binData, desc.raw->binSize);
          crypt->decrypt(*key, *data, *dBinDataString);
#ifdef _WIN32
          dBinDataPtr = (char*)::CoTaskMemAlloc(dBinDataString->size());
#else
          dBinDataPtr = new char[dBinDataString->size()];
#endif
          memcpy(dBinDataPtr, dBinDataString->data(), dBinDataString->size());
          desc.raw->binData = dBinDataPtr;
          desc.raw->binSize = dBinDataString->size();
        }

        delete data;
        delete crypt;
      }
      break;
    case ENC_ALGO_ECC571:
      {
      }
      break;
  }

  // TODO Uncompress message (ptr)

  disp->dispatch(desc, (char*)data, size);

  if (protDataPtr)
  {
#ifdef _WIN32
    ::CoTaskMemFree(protDataPtr);
#else
    delete [] protDataPtr;
#endif
  }

  if (xmlDataPtr)
  {
#ifdef _WIN32
    ::CoTaskMemFree(xmlDataPtr);
#else
    delete [] xmlDataPtr;
#endif
  }

  if (binDataPtr)
  {
#ifdef _WIN32
    ::CoTaskMemFree(binDataPtr);
#else
    delete [] binDataPtr;
#endif
  }

  if (dProtDataPtr)
  {
#ifdef _WIN32
    ::CoTaskMemFree(dProtDataPtr);
#else
    delete [] dProtDataPtr;
#endif
  }

  if (dXmlDataPtr)
  {
#ifdef _WIN32
    ::CoTaskMemFree(dXmlDataPtr);
#else
    delete [] dXmlDataPtr;
#endif
  }

  if (dBinDataPtr)
  {
#ifdef _WIN32
    ::CoTaskMemFree(dBinDataPtr);
#else
    delete [] dBinDataPtr;
#endif
  }

  delete dBinDataString;
  delete dXmlDataString;
  delete dProtDataString;
}

RawDecoder::RawDecoder(DataDispatcher * disp):
  _old(0),
  _dec3(0)
{
  _dec3 = new Decoder3(disp);
  sai::net::ProtocolDecoder *dec = new sai::net::ProtocolDecoder();
  _old = dec;
}

RawDecoder::~RawDecoder()
{
  sai::net::ProtocolDecoder * dec = (sai::net::ProtocolDecoder*) _old;
  delete dec;
  delete _dec3;
}

void 
RawDecoder::processData(Endpoint * endpoint, const char * data, const uint32_t size)
{
  if (size < 4)
  {
    return;
  }

  _dec3->lock();
  uint32_t magic;
  memcpy(&magic, data, sizeof(magic));
  magic = ntohl(magic);
  const char * ptr = data + sizeof(magic);
  if (magic == 0x000cdb1c)
  {
    uint16_t version;
    memcpy(&version, ptr, sizeof(version));
    version = ntohs(version);
    switch(version)
    {
      case 1:
        {
          std::string dat;
          dat.append(data, size);
          sai::net::DataDescriptor desc;
          sai::net::ProtocolDecoder *dec = (sai::net::ProtocolDecoder*) _old;
          dec->processDataEvent(desc, dat);
        }
        break;
      case 3:
        _dec3->processData(endpoint, data, size);
        break;
    }
  }
  _dec3->unlock();
}

