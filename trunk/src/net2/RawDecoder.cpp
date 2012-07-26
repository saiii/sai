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

#include <cstdio>
#include <string.h>
#include <cryptlib.h>
#include <sha.h>
#include <net2/DataDescriptor.h>
#include <net2/DataDispatcher.h>
#include <net/ProtocolDecoder.h>
#include <net2/RawDecoder.h>

using namespace sai::net2;

namespace sai
{
namespace net2
{

class Decoder2
{
private:
  DataDispatcher * disp;

public:
  Decoder2(DataDispatcher * d);
  ~Decoder2();

  void processData(const char * data, const uint32_t size);
};

}}

Decoder2::Decoder2(DataDispatcher * d):
  disp(d)
{}

Decoder2::~Decoder2()
{}

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
Decoder2::processData(const char * data, const uint32_t size)
{
  if (size < 12) return;

  // Magic and version are already verified so skip it
  char * ptr = (char *) (data + 6);

  DataDescriptor desc;
  desc.version = 2;

  desc.raw.r2.encAlgo = 0;
  READ(desc.raw.r2.encAlgo);
  READ(desc.raw.r2.encTagSize);
  READ(desc.raw.r2.comAlgo);
  READ(desc.raw.r2.comTagSize);
  READ(desc.raw.r2.hashAlgo);
  READ(desc.raw.r2.hashTagSize);
  READ(desc.raw.r2.protSize);
  READ(desc.raw.r2.xmlSize);
  READ(desc.raw.r2.binSize);

  if (desc.raw.r2.protSize > 0) desc.raw.r2.protSize = ntohl(desc.raw.r2.protSize);
  if (desc.raw.r2.xmlSize > 0)  desc.raw.r2.xmlSize  = ntohl(desc.raw.r2.xmlSize);
  if (desc.raw.r2.binSize > 0)  desc.raw.r2.binSize  = ntohl(desc.raw.r2.binSize);

  if (desc.raw.r2.encTagSize > 0)  READN(desc.raw.r2.encTag,  desc.raw.r2.encTagSize); 
  if (desc.raw.r2.comTagSize > 0)  READN(desc.raw.r2.comTag,  desc.raw.r2.comTagSize);
  if (desc.raw.r2.hashTagSize > 0) READN(desc.raw.r2.hashTag, desc.raw.r2.hashTagSize);

  switch(desc.raw.r2.hashAlgo)
  {
    case HASH_ALGO_SHA256:
    {
      CryptoPP::SHA256 sha256;
      CryptoPP::SecByteBlock digest256(sha256.DigestSize());
      sha256.Update((const byte*)ptr, desc.raw.r2.protSize + desc.raw.r2.xmlSize + desc.raw.r2.binSize);
      sha256.Final(digest256);

      uint8_t* ptr1 = (uint8_t*)desc.raw.r2.hashTag;
      for(uint32_t i = 0; i < sha256.DigestSize(); i += 1, ptr1++)
      {
        if (*ptr1 != digest256[i]) return;
      }
    }
    break;
  }

  char * protDataPtr, * xmlDataPtr, * binDataPtr;
  protDataPtr = xmlDataPtr = binDataPtr = 0;

  if (desc.raw.r2.protSize > 0)
  {
    desc.raw.r2.protData = new char[desc.raw.r2.protSize];
    READN(desc.raw.r2.protData, desc.raw.r2.protSize);
    protDataPtr = desc.raw.r2.protData;
  }

  if (desc.raw.r2.xmlSize > 0)
  {
#ifdef _WIN32
	desc.raw.r2.xmlData = (char*)::CoTaskMemAlloc(desc.raw.r2.xmlSize);
#else
    desc.raw.r2.xmlData = new char[desc.raw.r2.xmlSize];
#endif
    READN(desc.raw.r2.xmlData, desc.raw.r2.xmlSize);
    xmlDataPtr = desc.raw.r2.xmlData;
  }

  if (desc.raw.r2.binSize > 0)
  {
#ifdef _WIN32
	desc.raw.r2.binData = (char*)::CoTaskMemAlloc(desc.raw.r2.binSize);
#else
    desc.raw.r2.binData = new char[desc.raw.r2.binSize];
#endif
    READN(desc.raw.r2.binData, desc.raw.r2.binSize);
    binDataPtr = desc.raw.r2.binData;
  }

  // TODO Decrypt message (ptr)
  // TODO Uncompress message (ptr)

  disp->dispatch(desc);

  if (protDataPtr)
  {
    delete [] protDataPtr;
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
}

RawDecoder::RawDecoder(DataDispatcher * disp):
  _old(0),
  _dec2(0)
{
  _dec2 = new Decoder2(disp);
  sai::net::ProtocolDecoder *dec = new sai::net::ProtocolDecoder();
  _old = dec;
}

RawDecoder::~RawDecoder()
{
  sai::net::ProtocolDecoder * dec = (sai::net::ProtocolDecoder*) _old;
  delete dec;
  delete _dec2;
}

void 
RawDecoder::processData(const char * data, const uint32_t size)
{
  if (size < 4)
  {
    return;
  }

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
      case 2:
        _dec2->processData(data, size);
        break;
    }
  }
}

