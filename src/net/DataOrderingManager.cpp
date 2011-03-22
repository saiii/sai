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

DataOrderingManager  *DataOrderingManager::_instance = 0;

DataOrderingManager::DataOrderingManager(DataDispatchable* disp):
  _dispatcher(disp)
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

  _outgoingTable.clear();
}

void 
DataOrderingManager::addOutgoingData(DataDescriptor& desc, std::string data)
{
  DataBuffer * buffer = new DataBuffer();
  buffer->_time  = time(0);
  memcpy(&buffer->_desc, &desc, sizeof(DataDescriptor));
  buffer->_data.append(data.data(), data.size());

  _outgoingTable.insert(std::make_pair(desc.id, buffer)); 
  _outgoingList.push_back(buffer);
}

void 
DataOrderingManager::timerEvent()
{
  const int MAX_WAITING_TIME = 180;
  const int MAX_WAITING_ITEM = 100000;

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

  schedule();
}

bool
DataOrderingManager::check(DataDescriptor& desc, std::string data)
{
  const bool DISCARD_DATA = false;
  const bool ACCEPT_DATA  = true;

  // Is this a known sender?
  char txt[32] = {0};
  memset(txt, 0, sizeof(txt));
  memcpy(txt, desc.sender, sizeof(desc.sender));
  uint64_t desc_sender = strtoul(txt, 0, 16); 
  SenderTableIterator iter = _senderTable.find(desc_sender);
  if (iter == _senderTable.end())
  {
    // No, this is a new sender
    // Create sender profile
    SenderProfile * sender = new SenderProfile(_dispatcher);
    memcpy(&sender->_desc, &desc, sizeof(desc));
    _senderTable.insert(std::make_pair(desc_sender, sender));

    // Set expected message id from the first message we received
    sender->_expectedId = desc.id + 1;
    sender->_lastId     = desc.id;
    if (sender->_expectedId > 0xFFFFFFF0)
    {
      sender->_expectedId = 1;
    }
    sender->saveMessage(desc.id);
    return ACCEPT_DATA;
  }
  else
  {
    // Yes, we have received some thing from this guy before
    SenderProfile * sender = iter->second;
    if (sender->_expectedId == desc.id)
    {
      // The incoming message matches the expected message id
      sender->_expectedId = desc.id + 1;
      sender->_lastId     = desc.id;
      if (sender->_expectedId > 0xFFFFFFF0)
      {
        sender->_expectedId = 1;
      }
      sender->saveMessage(desc.id);
      sender->releaseMessage();
      return ACCEPT_DATA;
    }
    else
    {
      // It's not what we are looking for
      // have we already received this message?
      if (sender->hasMessage(desc.id))
      {
        // Yes, this is not the first time we received this one.
        return DISCARD_DATA;
      }
      else
      {
        // No
        // save it and request for the first missing one
        sender->saveMessage(desc.id);
        sender->saveMessage(desc, data);
        sender->requestMissingMessages(desc.id, desc.from.ival);
        return DISCARD_DATA;
      }
    }
  }
}

void 
DataOrderingManager::processDataRequest (DataDescriptor& desc, std::string& msg)
{
  // check id from outgoingTable
  // if not found -> send NOT FOUND to make the receiver reset its expected id
  // if found -> reply
  uint32_t from,to;
  char * ptr = (char*)msg.data();
  memcpy(&from, ptr, sizeof(from)); ptr += sizeof(from);
  memcpy(&to, ptr, sizeof(to));
  from = ntohl(from);
  to   = ntohl(to);

  DataDescriptor dc;
  dc.version   = 1;
  memcpy(dc.sender, Net::GetInstance()->getSenderId(), sizeof(dc.sender));
  dc.id        = 0;
  dc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
  dc.to.ival   = desc.from.ival;

  do 
  {
    dc.id = from;

    BufferTableIterator iter;
    if ((iter = _outgoingTable.find(from)) == _outgoingTable.end()) 
    {
      std::string msg = ":(";
      DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REP_OPCODE, dc, msg);
      return;
    }
    else
    {
      DataBuffer * buffer = iter->second;
      DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REP_OPCODE, dc, buffer->_data);
    }
  }while(from++ < to);
}

void 
DataOrderingManager::processDataResponse(DataDescriptor& desc, std::string& msg)
{
  char txt[32] = {0};
  memset(txt, 0, sizeof(txt));
  memcpy(txt, desc.sender, sizeof(desc.sender));
  uint64_t desc_sender = strtoul(txt, 0, 16);
  SenderTableIterator iter = _senderTable.find(desc_sender);
  if (iter != _senderTable.end())
  {
    SenderProfile * sender = iter->second;
    if (msg.compare(":(") == 0)
    {
      // if sender says "NOT FOUND"
      // flush what we have and reset the expectedId
      sender->flushMessage();
    }
    else
    {
      sender->saveMessage(desc.id);
      sender->saveMessage(desc, msg);
      sender->releaseMessage();
    }
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

SenderProfile::SenderProfile(DataDispatchable* disp):
  _flushMode(false),
  _sender(0),
  _time(0),
  _expectedId(0),
  _lastId(0),
  _dispatcher(disp)
{
}

SenderProfile::~SenderProfile()
{
  BufferTableIterator iter;
  while (_dataTable.size() > 0)
  {
    iter = _dataTable.begin();
    DataBuffer* buffer = iter->second;
    _dataTable.erase(iter);
    delete buffer;
  }
}

bool 
SenderProfile::hasMessage(uint32_t id)
{
  IntListIterator it = std::find(_intList.begin(), _intList.end(), id);
  if (it != _intList.end())
  {
    return true;
  }

  BufferTableIterator iter = _dataTable.find(id);
  return (iter == _dataTable.end())? false : true;
}

void 
SenderProfile::saveMessage(uint32_t id)
{
  _intList.push_back(id);
}

void 
SenderProfile::saveMessage(DataDescriptor& desc, std::string data)
{
  BufferTableIterator iter;
  if ((iter = _dataTable.find(desc.id)) != _dataTable.end())
  {
    DataBuffer * buffer = iter->second;
    memcpy(&buffer->_desc, &desc, sizeof(DataDescriptor));
    buffer->_data.clear();
    buffer->_data.append(data.data(), data.size());
  }
  else
  {
    DataBuffer * buffer = new DataBuffer();
    buffer->_time  = time(0);
    memcpy(&buffer->_desc, &desc, sizeof(DataDescriptor));
    buffer->_data.append(data.data(), data.size());

    _dataTable.insert(std::make_pair(desc.id, buffer));
  }
}

void 
SenderProfile::releaseMessage()
{
  _flushMode = false;
  schedule(0, 10);
}

void 
SenderProfile::flushMessage()
{
  _flushMode = true;
  schedule(0, 10);
}

void 
SenderProfile::requestMissingMessages(uint32_t currId, uint32_t to)
{
  DataDescriptor dc;
  dc.version   = 1;
  memcpy(dc.sender, Net::GetInstance()->getSenderId(), sizeof(dc.sender));
  dc.id        = 0;
  dc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
  dc.to.ival   = to;

  std::string msg;
  uint32_t reqId;
  char tmp[16];

  currId--;
  if (_expectedId > currId)
  {
    reqId = htonl(_expectedId);
    memcpy(tmp, &reqId, sizeof(reqId));
    msg.append(tmp, sizeof(reqId));

    reqId = 0xFFFFFFF0; 
    reqId = htonl(reqId);
    memcpy(tmp, &reqId, sizeof(reqId));
    msg.append(tmp, sizeof(reqId));

    DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, msg);
    msg.clear();

  
    reqId = 0;
    memcpy(tmp, &reqId, sizeof(reqId));
    msg.append(tmp, sizeof(reqId));

    reqId = htonl(currId);
    memcpy(tmp, &reqId, sizeof(reqId));
    msg.append(tmp, sizeof(reqId));
    DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, msg);
  }
  else
  {
    reqId = htonl(_expectedId);
    memcpy(tmp, &reqId, sizeof(reqId));
    msg.append(tmp, sizeof(reqId));

    reqId = htonl(currId);
    memcpy(tmp, &reqId, sizeof(reqId));
    msg.append(tmp, sizeof(reqId));
    DataBus::GetInstance()->send(SAI_DOMAIN, SAI_REQ_OPCODE, dc, msg);
  }
}

void 
SenderProfile::timerEvent()
{
  // send the buffered data out if its id matches the expected id 
  if (_dataTable.size() > 0)
  {
    if (!_flushMode)
    {
      BufferTableIterator iter = _dataTable.find(_expectedId);
      if (iter == _dataTable.end())
      {
        return;
      }

      DataBuffer * buffer = iter->second;
      _dataTable.erase(iter);
      _dispatcher->dispatch(buffer->_desc.opcode, buffer->_desc, buffer->_data);
      delete buffer;
    }
    else
    {
      BufferTableIterator iter;
      do 
      { 
        iter = _dataTable.find(_expectedId);
        _expectedId += 1;
        if (_expectedId > 0xFFFFFFF0)
        {
          _expectedId = 1;
        }
      } while(iter == _dataTable.end());

      if (iter != _dataTable.end())
      {
        DataBuffer * buffer = iter->second;
        _dataTable.erase(iter);
        _dispatcher->dispatch(buffer->_desc.opcode, buffer->_desc, buffer->_data);
        delete buffer;
      }
    }
    schedule(0, 10);
  }
}

