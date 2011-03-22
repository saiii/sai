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
#include <iostream>
#include <net/Net.h>
#include "ProtocolDecoder.h"

using namespace sai::net;

uint32_t
ProtocolDecoder::MagicToken::decode(DataDescriptor& desc, std::string& data)
{
  const uint32_t MAGIC = 0x000cdb1c;
  uint32_t magic = 0;
  uint32_t data_size = data.size();
  if (data_size < sizeof(magic)) 
  {
    return 0;
  }

  const char * ptr = data.c_str();
  memcpy(&magic, ptr, sizeof(magic));
  magic = ntohl(magic); 
  if (magic == MAGIC) 
  {
    ptr       += sizeof(magic);
    data_size -= sizeof(magic);
    std::string tdata;
    tdata.append(ptr, data_size);
    data = tdata; 
    return 1;
  }

  return 0;
}

uint32_t
ProtocolDecoder::VersionToken::decode(DataDescriptor& desc, std::string& data)
{
  uint16_t version = 0;
  const char * ptr = data.c_str();
  uint32_t data_size = data.size();
  if (data_size < sizeof(version)) 
  {
    return 0;
  }

  memcpy(&version, ptr, sizeof(version));
  version = ntohs(version);
  switch (version)  
  {
    case 1:
      {
        ptr       += sizeof(version);
        data_size -= sizeof(version);
        uint16_t flag = 0;
        if (data_size < sizeof(flag)) 
        {
          return 0;
        }

        memcpy(&flag, ptr, sizeof(flag));
        ptr       += sizeof(flag);
        data_size -= sizeof(flag);
        std::string tdata;
        tdata.append(ptr, data_size);
        data = tdata;
        flag = ntohs(flag);
        if (flag == 0)
        {
          desc.version = version;
          return 1;
        }
        else
        {
          return 0;
        }
      }
      break;
    default:
      return 0;
  }
}

uint32_t
ProtocolDecoder::SenderToken::decode(DataDescriptor& desc, std::string& data)
{
  uint32_t data_size = data.size();
  if (data_size < sizeof(desc.sender)) 
  {
    return 0;
  }

  const char * ptr = data.c_str();
  memcpy(desc.sender, ptr, sizeof(desc.sender)); 

  if (memcmp(desc.sender, Net::GetInstance()->getSenderId(), sizeof(desc.sender)) != 0)
  {
    ptr       += sizeof(desc.sender);
    data_size -= sizeof(desc.sender);
    std::string tdata;
    tdata.append(ptr, data_size);
    data = tdata; 
    return 1;
  }
  memset(desc.sender, 0, sizeof(desc.sender));

  return 0;
}

uint32_t
ProtocolDecoder::IdToken::decode(DataDescriptor& desc, std::string& data)
{
  uint32_t id = 0;
  uint32_t data_size = data.size();
  if (data_size < sizeof(desc.id)) 
  {
    return 0;
  }

  const char * ptr = data.c_str();
  memcpy(&id, ptr, sizeof(desc.id)); 
  desc.id = ntohl(id);

  ptr       += sizeof(desc.id);
  data_size -= sizeof(desc.id);
  std::string tdata;
  tdata.append(ptr, data_size);
  data = tdata; 
  return 1;
}

uint32_t
ProtocolDecoder::FromToToken::decode(DataDescriptor& desc, std::string& data)
{
  const uint16_t NAME = 0x0001;
  const uint16_t IPv4 = 0x0002; 

  uint16_t flagFrom, flagTo;
  flagFrom = flagTo = 0;

  const char * ptr = data.c_str();
  uint32_t data_size = data.size();
  if (data_size < sizeof(flagFrom)) 
  {
    return 0;
  }

  memcpy(&flagFrom, ptr, sizeof(flagFrom));
  ptr       += sizeof(flagFrom);
  data_size -= sizeof(flagFrom);
  flagFrom = ntohs(flagFrom);

  if (flagFrom & IPv4)
  {
    uint32_t ip = 0;
    if (data_size < sizeof(ip)) 
    {
      return 0;
    }

    memcpy(&ip, ptr, sizeof(ip));
    ptr       += sizeof(ip);
    data_size -= sizeof(ip);
    ip = ntohl(ip);

    desc.from.ival = ip;
  }
  else if (flagFrom & NAME)
  {
    char name[256];
    if (data_size < sizeof(name))
    {
      return 0;
    }

    memcpy(name, ptr, sizeof(name));
    ptr       += sizeof(name);
    data_size -= sizeof(name);
    name[255] = 0;

    desc.from.str = name;
  }
  else
  {
    return 0;
  }

  if (data_size < sizeof(flagTo))
  {
    return 0;
  }

  memcpy(&flagTo, ptr, sizeof(flagTo));
  ptr       += sizeof(flagTo);
  data_size -= sizeof(flagTo);
  flagTo = ntohs(flagTo);

  if (flagTo & IPv4)
  {
    uint32_t ip = 0;
    if (data_size < sizeof(ip))
    {
      return 0;
    }

    memcpy(&ip, ptr, sizeof(ip));
    ptr       += sizeof(ip);
    data_size -= sizeof(ip);
    ip = ntohl(ip);

    desc.to.ival = ip;
  }
  else if (flagTo & NAME)
  {
    char name[256];
    if (data_size < sizeof(name))
    {
      return 0;
    }

    memcpy(name, ptr, sizeof(name));
    ptr       += sizeof(name);
    data_size -= sizeof(name);
    name[255] = 0;

    desc.to.str = name;
  }
  else
  {
    return 0;
  }

  std::string tdata;
  tdata.append(ptr, data_size);
  data = tdata;
  return 1;
}

uint32_t
ProtocolDecoder::Data::decode(DataDescriptor& desc, std::string& data)
{
  uint32_t id = 0;
  const char * ptr = data.c_str();
  uint32_t data_size = data.size();

  if (data_size < sizeof(id))
  {
    return 0;
  }

  memcpy(&id, ptr, sizeof(id));
  ptr       += sizeof(id);
  data_size -= sizeof(id);
  id = ntohl(id);
  desc.opcode = id;

  std::string tdata;
  tdata.append(ptr, data_size);
  data = tdata;
  return id;
}

void 
ProtocolDecoder::DefaultDataHandler::processDataEvent(DataDescriptor& desc, std::string&)
{
  std::string frmString;
  desc.from.toString(frmString, true);
  std::cerr << "(DefaultDataHandler) Receive unknown opcode from " << frmString << std::endl;
}

ProtocolDecoder::ProtocolDecoder():
  _defaultDataHandler(0)
{
  _magic.registerHandler(1, &_version);
  _version.registerHandler(1, &_sender);
  _sender.registerHandler(1, &_id);
  _id.registerHandler(1, &_fromTo);
  _fromTo.registerHandler(1, &_data);

  _defaultDataHandler = new DefaultDataHandler();
  _data.setDefaultHandler(_defaultDataHandler);
}

ProtocolDecoder::~ProtocolDecoder()
{
  delete _defaultDataHandler;
}

void 
ProtocolDecoder::processDataEvent(DataDescriptor& desc, std::string& data)
{
  _magic.processDataEvent(desc, data);
}

bool 
ProtocolDecoder::registerHandler(uint32_t id, DataHandler * handler)
{
  return _data.registerHandler(id, handler);
}

void 
ProtocolDecoder::setDefaultHandler(DataHandler * handler)
{
  _data.setDefaultHandler(handler);
}

