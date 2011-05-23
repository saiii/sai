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

#include <algorithm>
#include <net/DataBus.h>
#include "DataOrderingManager.h"

using namespace sai::net;

#define SAI_DOMAIN           "<SAI<ORDERING>>"
#define SAI_REQ_OPCODE       0xFFFFFFF0
#define SAI_REP_OPCODE       0xFFFFFFF1

class Req : public DataHandler
{
public:
  DataOrderingManager * manager;

public:
  Req() : manager(0) {}
  virtual ~Req() {}

  void processDataEvent(DataDescriptor& desc, std::string& data)
  {
    manager->processReqtEvent(desc, data);
  }
};

class Resp : public DataHandler
{
public:
  DataOrderingManager * manager;

public:
  Resp() : manager(0) {}
  virtual ~Resp() {}

  void processDataEvent(DataDescriptor& desc, std::string& data)
  {
    manager->processRespEvent(desc, data);
  }
};

DataOrderingManager  *DataOrderingManager::_instance = 0;

DataOrderingManager* 
DataOrderingManager::GetInstance()
{
  if (!_instance)
  {
    _instance = new DataOrderingManager();
  }
  return _instance;
}

DataOrderingManager::DataOrderingManager():
  _req(0),
  _resp(0)
{
  _req  = new Req();
  _resp = new Resp();

  _repeater.list    = &_outgoingList;
  _repeater.manager = this;
}

void 
DataOrderingManager::initialize()
{
  DataBus * bus = DataBus::GetInstance();

  bus->listen(SAI_DOMAIN);
  bus->registerHandler(SAI_REQ_OPCODE, _req);
  bus->registerHandler(SAI_REP_OPCODE, _resp);

  schedule(2, 0);
}

DataOrderingManager::~DataOrderingManager()
{
  _outgoingList.clear();

  delete _resp;
  delete _req;
}

void 
DataOrderingManager::timerEvent()
{
  // remove expiring message from the outgoing list & table
  time_t t = time(0);
  for (uint32_t i = 0; i < _outQueue.size(); i += 1)
  {
    OutputPacket * packet = dynamic_cast<OutputPacket*>(_outQueue.at(i));
    if (t >= packet->t)
    {
      _outQueue.removeAt(i);
      schedule(0, 1);
      return;
    }
  }

  schedule();
}

void 
DataOrderingManager::processReqtEvent(DataDescriptor& desc, std::string& data)
{
  // TODO
  // call request
}

void 
DataOrderingManager::processRespEvent(DataDescriptor& desc, std::string& data)
{
  // TODO
  // send the missing packet to the main flow
}

void 
DataOrderingManager::save(uint32_t id, const char * data, uint32_t sz)
{
  while (_outQueue.size() > 1000000) 
  {
    _outQueue.remove(0);
  }

  OutputPacket * packet = new OutputPacket();
  packet->data.append(data, sz);
  packet->pktId   = id;
  packet->t       = time(0) + 300;
  packet->reqs    = 0;
  packet->pending = 0;

  _outQueue.add(id, packet);
}

bool 
DataOrderingManager::receive(uint32_t from, uint32_t opcode, DataDescriptor& desc, std::string data)
{
  enum { GOOD_TO_GO = true, WAIT = false };

  SenderTableIterator iter = _senderTable.find(from);
  if (iter == _senderTable.end())
  {
    SenderProfile * sender = new SenderProfile(); // TODO : When to delete this?
    sender->name       = from;
    sender->t          = time(0) + 3600;
    sender->expectedId = desc.id + 1;
    if (sender->expectedId > 0xFFFFFFF0);
    {
      sender->expectedId = 0;
    }
    _senderTable.insert(std::make_pair(from, sender));
    return GOOD_TO_GO;
  }

  SenderProfile *sender = iter->second;
  sender->t = time(0) + 3600;

  if (sender->expectedId == 0 || desc.id == sender->expectedId)
  {
    sender->expectedId = desc.id + 1;
    if (sender->expectedId > 0xFFFFFFF0);
    {
      sender->expectedId = 0;
    }

    if (sender->missingList.size() > 0)
    {
      // TODO : Should we clear all pending items because we already have the expected one?
    }

    if (sender->inQueue.size() > 0) // Ex. expecting 3, 3 has come, so release the buffered 1 and 2
    {
      // TODO
    }
    return GOOD_TO_GO;
  }

  if (sender->expectedId < desc.id) // waiting for 5 but 10 has come
  {
    for (uint32_t i = sender->expectedId; i < desc.id; i += 1)
    {
      sender->missingList.push_back(i);
    }  

    // Keep the 10
    InputPacket * packet = new InputPacket();
    packet->from    = from;
    DataDescriptor::Copy(packet->desc, desc);
    packet->data.append(data.data(), data.size());
    packet->pktId   = desc.id;
    packet->t       = time(0) + 3600;
    sender->inQueue.add(desc.id, packet);
    return WAIT;
  }
  else // waiting for 100 but 2 has come
  {
    // it's possible that the message has already overflown and started at 1 again
    // or
    // it's just a duplicate and faulty message
    return WAIT; // simply ignore it. moreover, we dont need to keep this one
  }
}

void
DataOrderingManager::request(uint32_t id, std::string from)
{
  OutputPacket * pkt = dynamic_cast<OutputPacket*>(_outQueue.get(id));
  if (!pkt)
  {
    return;
  }

  if (pkt->pending || ++pkt->reqs > 1000)
  {
    return;
  }

  if (_outgoingList.size() == 0)
  {
    _repeater.schedule(0, 100);
  }

  pkt->pending = 1;
  pkt->to      = from;
  _outgoingList.push_back(pkt);
}

void 
DataOrderingManager::MsgRepeater::timerEvent()
{
  OutputPacket * packet = dynamic_cast<OutputPacket*>(list->front());
  list->erase(list->begin());
  manager->send(packet->to, packet->pktId, packet->data, false);
  packet->pending = 0;
  if (list->size() > 0)
  {
    schedule(0, 1);
  }
}

void 
DataOrderingManager::send(std::string to, uint32_t id, std::string data, bool save)
{
  DataBus::GetInstance()->send(to, id, data, save);
}

SenderProfile::SenderProfile():
  t(0),
  expectedId(0),
  name(0)
{
}

SenderProfile::~SenderProfile()
{
}
