//=============================================================================
// Copyright (C) 2011 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <time.h>
#include <string.h>
#include <net/DataBus.h>
#include "DataOrderingManager.h"

using namespace sai::net;

#define SAI_DOMAIN           "<SAI<ORDERING>>"
#define SAI_REQ_OPCODE       0xFFFFFFF0
#define SAI_REP_OPCODE       0xFFFFFFF1

DataOrderingManager::DataOrderingManager(DataDispatchable* disp):
  _dispatcher(disp),
  _expectedId(0),
  _lastId(0)
{
  _requestHandler.manager  = this;
  _responseHandler.manager = this;
}

void 
DataOrderingManager::initialize()
{
  DataBus * bus = DataBus::GetInstance();

  bus->listen(SAI_DOMAIN);
  bus->registerHandler(SAI_REQ_OPCODE, &_requestHandler);
  bus->registerHandler(SAI_REP_OPCODE, &_responseHandler);

  schedule(0, 800);
}

DataOrderingManager::~DataOrderingManager()
{
  while(_outgoingList.size() > 0)
  {
    DataBuffer* buffer = _outgoingList.front();
    _outgoingList.erase(_outgoingList.begin());
    delete buffer;
  }

  while(_incomingList.size() > 0)
  {
    DataBuffer* buffer = _incomingList.front();
    _outgoingList.erase(_incomingList.begin());
    delete buffer;
  }

  _outgoingTable.clear();
  _incomingTable.clear();
}

void 
DataOrderingManager::addOutgoingData(DataDescriptor& desc, std::string data)
{
  DataBuffer * buffer = new DataBuffer();
  buffer->_time  = time(0);
  buffer->_state = SENT;
  memcpy(&buffer->_desc, &desc, sizeof(DataDescriptor));
  buffer->_data.append(data.data(), data.size());

  _outgoingTable.insert(std::make_pair(desc.id, buffer)); 
  _outgoingList.push_back(buffer);
}

void 
DataOrderingManager::addPendingRequest(DataDescriptor& desc)
{
  DataBuffer * buffer = new DataBuffer();
  buffer->_time  = 0;
  buffer->_state = PENDING_REQUEST;
  memcpy(&buffer->_desc, &desc, sizeof(DataDescriptor));

  _incomingTable.insert(std::make_pair(desc.id, buffer)); 
  _incomingList.push_back(buffer);
}

void 
DataOrderingManager::addIncomingData(DataDescriptor& desc, std::string data)
{
  BufferTableIterator iter;
  if ((iter = _incomingTable.find(desc.id)) != _incomingTable.end())
  {
    DataBuffer * buffer = iter->second;
    memcpy(&buffer->_desc, &desc, sizeof(DataDescriptor));
    buffer->_state = READY;
    buffer->_data.clear();
    buffer->_data.append(data.data(), data.size());
  }
}

void 
DataOrderingManager::timerEvent()
{
  const int MAX_WAITING_TIME = 180;
  const int MAX_WAITING_ITEM = 1000000;
  const int MAX_REQUEST_TIMES= 3;

  if (_outgoingList.size() > 0)
  {
    uint32_t now = time(0);
    DataBuffer * buffer = _outgoingList.front();
    if ((now - buffer->_time) >= MAX_WAITING_TIME)
    {
      BufferListIterator iter;
      for (iter  = _outgoingList.begin();
           iter != _outgoingList.end();
           iter ++) 
      {
        buffer = *iter;
        if ((now - buffer->_time) >= MAX_WAITING_TIME)
        {
          _outgoingList.erase(_outgoingList.begin());
          delete buffer;
          iter = _outgoingList.begin();
          continue;
        }
      }
    }

    while(_outgoingList.size() > MAX_WAITING_ITEM)
    {
      buffer = _outgoingList.front();
      _outgoingList.erase(_outgoingList.begin());
      delete buffer;
    }
  }

  if (_incomingList.size() > 0)
  {
    // if the first one is ready then release it
    DataBuffer * buffer = 0;

    while (_incomingList.size() > 0)
    {
      buffer = _incomingList.front();
      if (buffer->_state == READY)
      {
        _dispatcher->dispatch(buffer->_desc.id, buffer->_desc, buffer->_data, false);
        _incomingList.erase(_incomingList.begin());
        delete buffer;
      }
      else
      {
        break;
      }
    }
  
    if (_incomingList.size() > 0)
    {
      bool requestNeeded = true;

      // Try to request the next pending item
      BufferListIterator iter;
      for (iter  = _incomingList.begin();
           iter != _incomingList.end();
           iter ++)
      {
        buffer = *iter;
        if (buffer->_state == PENDING_REQUEST)
        {
          buffer->_time += 1;
          buffer->_state = REQUESTED;
          requestNeeded = false;

          // Re-request
          DataDescriptor dc;
          dc.version   = 1;
          memcpy(dc.sender, Net::GetInstance()->getSenderId(), sizeof(dc.sender));
          dc.id        = 0;
          dc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
          dc.to.ival   = buffer->_desc.from.ival;
          std::string msg;
          uint32_t reqId = htonl(buffer->_desc.id);
          char tmp[16];
          memcpy(tmp, &reqId, sizeof(reqId));
          msg.append(tmp, sizeof(reqId));
          DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, msg);
          break; // just one is enough
        }
      }

      // if all of the pending items are requested
      // then try to make a re-request from the begining 
      // if the retry times reaches 3, then remove all items
      // and accept the missing data
      if (requestNeeded)
      {
        buffer = _incomingList.back();
        uint32_t cnt = buffer->_time; 
        if (cnt == MAX_REQUEST_TIMES)
        {
          while(_incomingList.size() > 0)
          {
            buffer = _incomingList.front();
            if (buffer->_state == READY)
            {
              _dispatcher->dispatch(buffer->_desc.id, buffer->_desc, buffer->_data, false);
            }
            _incomingList.erase(_incomingList.begin());
            // set expect id to 0 to accept all data
            _expectedId = _lastId = 0;
            delete buffer;
          }
        }
        else
        {
          for (iter  = _incomingList.begin();
               iter != _incomingList.end();
               iter ++)
          {
            buffer = *iter;
            if (buffer->_state == REQUESTED && buffer->_time <= cnt)
            {
              buffer->_time += 1;

              // Re-request
              DataDescriptor dc;
              dc.version   = 1;
              memcpy(dc.sender, Net::GetInstance()->getSenderId(), sizeof(dc.sender));
              dc.id        = 0;
              dc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
              dc.to.ival   = buffer->_desc.from.ival;
              std::string msg;
              uint32_t reqId = htonl(buffer->_desc.id);
              char tmp[16];
              memcpy(tmp, &reqId, sizeof(reqId));
              msg.append(tmp, sizeof(reqId));
              DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, msg);
              break; // just one item
            }
          }
        }
      }
    }
  }
  schedule();
}

bool
DataOrderingManager::check(DataDescriptor& desc, std::string data)
{
  const bool VALID  = true;
  const bool INVALID= false;

  if (desc.id == 0) // This is used by point-to-point message.
  {
    return VALID;
  }

  if (_expectedId == desc.id || _expectedId == 0)
  {
    _lastId     = desc.id;
    _expectedId = desc.id + 1;
    if (_expectedId > 0xFFFFFFF0)
    {
      _expectedId = 1;
    }
    return VALID;
  }

  DataDescriptor d;
  memcpy(&d, &desc, sizeof(d));

  if (desc.id < _expectedId) // the id is now back to 1
  {
    for (uint32_t id = _expectedId; id <= 0xFFFFFFF0; id += 1)
    {
      d.id = id;
      addPendingRequest(d);
    }

    for (uint32_t id = 1; id < desc.id; id += 1)
    {
      d.id = id;
      addPendingRequest(d);
    }
  }
  else
  {
    for (uint32_t id = _expectedId; id < desc.id; id += 1)
    {
      d.id = id;
      addPendingRequest(d);
    }
  }

  // Keep the data
  addIncomingData(desc, data);

  return INVALID;
}

void 
DataOrderingManager::processDataRequest (DataDescriptor& desc, std::string& msg)
{
  // check id from outgoingTable
  // if not found -> send NOT FOUND to make the receiver reset its expected id
  // if found -> reply

  BufferTableIterator iter;
  if ((iter = _outgoingTable.find(desc.id)) == _outgoingTable.end()) 
  {
    DataDescriptor dc;
    dc.version   = 1;
    memcpy(dc.sender, Net::GetInstance()->getSenderId(), sizeof(dc.sender));
    dc.id        = 0;
    dc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
    dc.to.ival   = desc.from.ival;
    std::string msg = "*";
    DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, msg);
  }
  else
  {
    DataBuffer * buffer = iter->second;

    DataDescriptor dc;
    dc.version   = 1;
    memcpy(dc.sender, Net::GetInstance()->getSenderId(), sizeof(dc.sender));
    dc.id        = 0;
    dc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
    dc.to.ival   = desc.from.ival;
    DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, buffer->_data);
  }
}

void 
DataOrderingManager::processDataResponse(DataDescriptor& desc, std::string& msg)
{
  BufferTableIterator iter;
  if ((iter = _incomingTable.find(desc.id)) == _incomingTable.end()) 
  {
    ; // just discard it
  }
  else
  {
    DataBuffer * buffer = iter->second;
    memcpy(&buffer->_desc, &desc, sizeof(desc));
    buffer->_state = READY;
    buffer->_data.clear();
    buffer->_data.append(msg.data(), msg.size());
  }
}

void 
DataRequestHandler::processDataEvent(DataDescriptor& desc, std::string& msg)
{
  manager->processDataRequest(desc, msg);
}

void 
DataResponseHandler::processDataEvent(DataDescriptor& desc, std::string& msg)
{
  manager->processDataResponse(desc, msg);
}

