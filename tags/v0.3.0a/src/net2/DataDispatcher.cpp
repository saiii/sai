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
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdlib.h>
#include <cstring>
#include <utils/XmlReader.h>
#include <net/Exception.h>
#include <net/ProtocolDecoder.h>
#include "DataDispatcher.h"

using namespace sai::net2;

DataDispatcher::DataDispatcher():
  _defaultHandler(0)
{
}

DataDispatcher::~DataDispatcher()
{
  _table.clear();
}

void 
DataDispatcher::dispatch(DataDescriptor& desc)
{
  std::string protData;
  protData.append(desc.raw.r2.protData, desc.raw.r2.protSize);
  char * protDataPtr = (char*)protData.data();
  uint16_t version = 0;
  memcpy(&version, protDataPtr, sizeof(version));
  version = ntohs(version);
  protDataPtr += sizeof(version);
  desc.raw.r2.protData = protDataPtr;

  sai::utils::XmlReader pReader;
  sai::utils::XmlReader dReader;
  switch(version)
  {
    case 1:
      {
        pReader.parseMem(desc.raw.r2.protData);
        pReader.moveTo("protocol");
        std::string uuid;
        pReader.get("uuid", "value", uuid);
        std::string myUUID;
        ProtMessage::getUUID(myUUID);
        if (strcmp(uuid.c_str(), myUUID.c_str()) == 0)
        {
          // This is our own message!
          return;
        }

        std::string sid;
        pReader.get("id", "value", sid);
        uint32_t id = atoi(sid.c_str());
        desc.xmlProtReader = &pReader;

        std::string xmlData;
        if (desc.raw.r2.xmlData)
        {
          xmlData.append(desc.raw.r2.xmlData, desc.raw.r2.xmlSize);
          desc.raw.r2.xmlData = (char*)xmlData.data();
          dReader.parseMem(desc.raw.r2.xmlData);
          desc.xmlDataReader = &dReader;
        }

        DispatchTableIterator iter;
        if ((iter = _table.find(id)) != _table.end())
        {
          DataHandler * handler = iter->second;
          handler->processDataEvent(desc);
          return;
        }
      }
      break;
    default:
      break;
  }

  if (_defaultHandler)
  {
    _defaultHandler->processDataEvent(desc);
  }
  else
  {
    throw sai::net::DataException("No suitable DataHandler for this opcode!");
  }
}

bool 
DataDispatcher::registerHandler(uint32_t id, DataHandler * handler)
{
  const bool NEW_ENTRY = true;
  const bool DUPLICATED = false;

  DispatchTableIterator iter = _table.find(id);
  if (iter != _table.end())
  {
    return DUPLICATED;
  }

  _table.insert(std::make_pair(id, handler));

  return NEW_ENTRY;
}

bool 
DataDispatcher::registerHandler(uint32_t id, sai::net::DataHandler * handler)
{
  if (_old)
  {
    sai::net::ProtocolDecoder * dec = (sai::net::ProtocolDecoder*) _old;
    return dec->registerHandler(id, handler);
  }
  return false;
}

void 
DataDispatcher::setDefaultHandler(DataHandler * handler)
{
  _defaultHandler = handler;
}

