//=============================================================================
// Copyright (C) 2009 Athip Rooprayochsilp <athipr@gmail.com>
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
#include <string.h>
#endif
#include "Exception.h"
#include "ProtocolEncoder.h"

using namespace sai::net;

#define MAGIC 0x000cdb1c

class _EncoderV1
{
public:
  _EncoderV1()
  {}
  ~_EncoderV1()
  {}
  void encode(DataDescriptor& desc, 
              uint32_t        id, 
              std::string& data, 
              std::string& ret);
};

ProtocolEncoder::ProtocolEncoder()
{
}

ProtocolEncoder::~ProtocolEncoder()
{
}

void 
ProtocolEncoder::encode(DataDescriptor& desc, 
                        uint32_t        id, 
                        std::string& data, 
                        std::string& ret)
{
  switch (desc.version)
  {
    case 1:
      _EncoderV1().encode(desc, id, data, ret);
      break;
    default:
      throw DataException("The specified version does not support!");
      break;
  }
}

#define ADD32(str,val) { uint32_t v = htonl(val); str.append((char*)&v, sizeof(v)); }
#define ADD16(str,val) { uint16_t v = htons(val); str.append((char*)&v, sizeof(v)); }
#define ADDSTR256(str,val) { char v[256]; memset(v, 0, sizeof(v)); memcpy(v, val.c_str(), val.length()); str.append(v, sizeof(v)); }

void 
_EncoderV1::encode(DataDescriptor& desc, 
                   uint32_t        id, 
                   std::string& data,
                   std::string& ret)
{
  ret.clear();

  ADD32(ret, MAGIC);
  ADD16(ret, 1);
  ADD16(ret, 0);
 
  if (desc.from.str.length() == 0)
  { 
    ADD16(ret, 2);
    ADD32(ret, desc.from.ival);
  }
  else
  {
    ADD16(ret, 1);
    ADDSTR256(ret, desc.from.str);
  } 

  if (desc.to.str.length() == 0)
  { 
    ADD16(ret, 2);
    ADD32(ret, desc.to.ival);
  }
  else
  {
    ADD16(ret, 1);
    ADDSTR256(ret, desc.to.str);
  } 

  ADD32(ret, id);
  ret.append(data.data(), data.size());
}

