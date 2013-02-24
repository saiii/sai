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
#include <net2/Transport.h>
#include "DataDispatcher.h"

using namespace sai::net2;

DataDispatcher::DataDispatcher():
  _defaultHandler(0),
  _ignoreDestinationField(false)
{
}

DataDispatcher::~DataDispatcher()
{
  _table.clear();
}

void 
DataDispatcher::dispatch(DataDescriptor& desc, char * origMsg, uint32_t origSize)
{
  std::string protData;
  protData.append(desc.raw->protData, desc.raw->protSize);
  char * protDataPtr = (char*)protData.data();
  uint16_t version = 0;
  memcpy(&version, protDataPtr, sizeof(version));
  version = ntohs(version);
  protDataPtr += sizeof(version);
  desc.raw->protSize -= sizeof(version);
  desc.raw->protData = protDataPtr;

  switch(version)
  {
    case 1:
      {
        //desc.raw->print();
        desc.protMessage->fromString(desc.raw->protData);
        std::string uuid;
        desc.protMessage->get("uuid", uuid);

        std::string myUUID;
        ProtMessage::getUUID(myUUID);
        if (strcmp(uuid.c_str(), myUUID.c_str()) == 0)
        {
          // This is our own message!
          return;
        }

        if (!_ignoreDestinationField)
        {
          std::string destination;
          desc.protMessage->get("destination", destination);
          if (destination.length() > 0 && strcmp(destination.c_str(), ".*") != 0)
          {
            if (strcmp(myUUID.c_str(), destination.c_str()) != 0)
            {
              // We are not support to process this message!
              return;
            }
          }
        }

        std::string sid;
        desc.protMessage->get("id", sid);
        uint32_t id = atoi(sid.c_str());

        std::string xmlData;
        if (desc.raw->xmlData)
        {
          xmlData.append(desc.raw->xmlData, desc.raw->xmlSize);
          desc.raw->xmlData = (char*)xmlData.data();
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
DataDispatcher::intlRegisterHandler(uint32_t id, DataHandler * handler)
{
  if (id > 1000)
  {
    throw sai::net::DataException("The id must be smaller than 1000!");
    return false;
  }

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
DataDispatcher::registerHandler(uint32_t id, DataHandler * handler)
{
  if (id <= 1000)
  {
    throw sai::net::DataException("The id must be greater than 1000!");
    return false;
  }

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

