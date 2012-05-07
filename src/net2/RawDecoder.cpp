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

#include <string.h>
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

  // Magic is already verified so skip it
  char * ptr = (char *) (data + 2);

  DataDescriptor desc;

  READ(desc.version);
  if (desc.version != 2)
  {
    return;
  }

  desc.raw.r2.encAlgo = 0;
  READ(desc.raw.r2.encAlgo);
  READ(desc.raw.r2.encTagSize);
  if (desc.raw.r2.encTagSize > 0) READN(desc.raw.r2.encTag, desc.raw.r2.encTagSize); 
  READ(desc.raw.r2.comAlgo);
  READ(desc.raw.r2.comTagSize);
  if (desc.raw.r2.comTagSize > 0) READN(desc.raw.r2.comTag, desc.raw.r2.comTagSize);
  READ(desc.raw.r2.xmlAndBinaryFlag);
  // 1 - XML
  // 2 - BIN
  READ(desc.raw.r2.xmlSize); desc.raw.r2.xmlSize = ntohl(desc.raw.r2.xmlSize);
  READ(desc.raw.r2.binSize); desc.raw.r2.binSize = ntohl(desc.raw.r2.binSize);

  // TODO Uncompress message (ptr)
  // TODO Decrypt message (ptr)

  if (desc.raw.r2.xmlAndBinaryFlag & 0x1)
  {
    if (desc.raw.r2.xmlSize > 0) 
    {
      desc.xml.x2.data = new char[desc.raw.r2.xmlSize];
      READN(desc.xml.x2.data, desc.raw.r2.xmlSize);
    }
  }

  if (desc.raw.r2.xmlAndBinaryFlag & 0x2)
  {
    READ(desc.binary.b2.num);
    desc.binary.b2.num = ntohs(desc.binary.b2.num);
    desc.binary.b2.map = new Binary2::Pair[desc.binary.b2.num];
    uint32_t total = 0;
    for (uint16_t i = 0; i < desc.binary.b2.num; i += 1)
    {
      READ(desc.binary.b2.map[i].offset);  
      READ(desc.binary.b2.map[i].size);  
      desc.binary.b2.map[i].offset = ntohl(desc.binary.b2.map[i].offset);
      desc.binary.b2.map[i].size   = ntohl(desc.binary.b2.map[i].size);
      total += desc.binary.b2.map[i].size;
    }
    if (total > 0) 
    {
      desc.binary.b2.data = new char[total];
      READN(desc.binary.b2.data, total); 
    }
  }

  disp->dispatch(desc);
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

  // check magic
  uint16_t magic2;  
  uint32_t magic1;
  
  memcpy(&magic2, data, sizeof(magic2));
  magic2 = ntohs(magic2);
  if (magic2 == 0xcdb1c)
  {
    _dec2->processData(data, size);
  }
  else // backward compatibility
  {
    memcpy(&magic1, data, sizeof(magic1));
    magic1 = ntohs(magic1);

    if (magic1 == 0x000cdb1c)    
    {
      std::string dat;
      dat.append(data, size);
      sai::net::DataDescriptor desc;
      sai::net::ProtocolDecoder *dec = (sai::net::ProtocolDecoder*) _old;
      dec->processDataEvent(desc, dat);
    }
  }
}

