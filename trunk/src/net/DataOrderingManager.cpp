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

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <net/DataBus.h>
#include "DataOrderingManager.h"

using namespace sai::net;

#define SAI_DOMAIN           "<SAI<ORDERING>>"
#define SAI_REQ_OPCODE       0xFFFFFFF0

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
  _req(0)
{
  Req * req = new Req();
  req->manager = this;
  _req         = req;

  _repeater.list    = &_outgoingList;
  _repeater.manager = this;
}

void 
DataOrderingManager::initialize()
{
  DataBus * bus = DataBus::GetInstance();

  bus->listen(SAI_DOMAIN);
  bus->registerHandler(SAI_REQ_OPCODE, _req);

  schedule(2, 0);
}

DataOrderingManager::~DataOrderingManager()
{
  _outgoingList.clear();

  delete _req;
}

void 
DataOrderingManager::timerEvent()
{
  time_t t = time(0);
  // remove expiring sender from the senderTable
  for (SenderTableIterator iter =  _senderTable.begin();
                           iter != _senderTable.end();
                           iter ++)
  {
    SenderProfile * sender = iter->second;
    if (t >= sender->t)
    {
      _senderTable.erase(iter);
      delete sender;
      if (_senderTable.size() > 0)
      {
        iter = _senderTable.begin();
      }
      else
      {
        break;
      }
    }
  }

  // remove expiring message from the outgoing list & table
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
  uint32_t id = atoi(data.c_str());

  std::string from;
  desc.from.toString(from, Address::RAW_MSG);
  request(id, from);
}

void 
DataOrderingManager::save(uint32_t id, uint32_t opcode, const char * data, uint32_t sz)
{
#if 0
  if (opcode == SAI_REQ_OPCODE)
  {
    return;
  }
#endif

  while (_outQueue.size() > 1000000) 
  {
    _outQueue.remove(0);
  }

  OutputPacket * packet = new OutputPacket();
  packet->data.append(data, sz);
  packet->pktId   = id;
  packet->opcode  = opcode;
  packet->t       = time(0) + 300;
  packet->reqs    = 0;
  packet->pending = 0;

  _outQueue.add(id, packet);
}

SenderProfile * 
DataOrderingManager::checkSender(DataDescriptor& desc)
{
  SenderProfile * ret = 0;
  char tSender[17];
  memcpy(tSender, desc.sender, 16);
  tSender[16] = 0;
  SenderTableIterator iter = _senderTable.find(tSender);
  if (iter == _senderTable.end())
  {
    SenderProfile * sender = new SenderProfile(); 
    sender->sender     = tSender;
    sender->t          = time(0) + 360;
    sender->expectedId = 0;
    if (sender->expectedId > 0xFFFFFFF0)
    {
      sender->expectedId = 0;
    }
    _senderTable.insert(std::make_pair(sender->sender, sender));
    ret = sender;
  }
  else
  {
    ret    = iter->second;
    ret->t = time(0) + 360;
  }

  return ret;
}

void 
SenderProfile::addMissingList(std::string name, uint32_t from, uint32_t to)
{
  bool emptyQueue = missingQueue.size() == 0 ? true : false;
  for (uint32_t v = from; v <= to; v += 1)
  {
    if (inQueue.get(v) != 0)
    {
      continue;
    }
  
    MissingPacket * packet = dynamic_cast<MissingPacket *>(missingQueue.get(v));
    if (!packet)
    {
      MissingPacket * packet = new MissingPacket();
      packet->pktId = v;
      packet->reqs  = 0;
      packet->name  = name;
      packet->t     = time(0);
      packet->opcode= 0;
      missingQueue.add(v, packet);
    }
  }

  if (emptyQueue && missingQueue.size() > 0)
  {
    schedule(0, 1);
  }
}

void 
SenderProfile::timerEvent()
{
  if (missingQueue.size() == 0)
  {
    return;
  }

  MissingPacket * packet = dynamic_cast<MissingPacket*>(missingQueue.popAndPutLast());
  if (!packet)
  {
    return;
  }

  if (packet->t >= time(0)) 
  {
    schedule(0, 500);
    return;
  }

  char buff[32];
  sprintf(buff, "%u", packet->pktId);
  DataBus::GetInstance()->send(packet->name, SAI_REQ_OPCODE, buff, false);
  packet->t = time(0) + 1;

  if (++packet->reqs > 10)
  {
    expectedId = packet->pktId + 1;
    if (expectedId > 0xFFFFFFF0)
    {
      expectedId = 1;
    }
    releaseMessage();
    missingQueue.remove(packet);
  }

  if (missingQueue.size() > 0)
  {
    schedule(0, 200);
  }
}

void 
SenderProfile::releaseMessage()
{
  // ACT#2
  // This can be enhanced by adding timer to prevent the long time processing loop
  uint32_t rFrom = expectedId;
  bool found = true;
  do 
  {
    if (inQueue.size() == 0) break;
    InputPacket * pkt = dynamic_cast<InputPacket*>(inQueue.get(rFrom));
    if (!pkt)
    {
      found = false;
    }
    else
    {
      DataBus::GetInstance()->getDataDecoder()->setReplay();
      DataBus::GetInstance()->getDataDecoder()->dispatch(pkt->opcode, pkt->desc, pkt->data);
      DataBus::GetInstance()->getDataDecoder()->clrReplay();
      inQueue.remove(pkt);

      rFrom += 1;
      if (rFrom > 0xFFFFFFF0)
      {
        rFrom = 1;
      }
    }
  }while (found);
  expectedId = rFrom;

  if (inQueue.size() == 0 && missingQueue.size() > 0)
  {
    missingQueue.clear();
  }
}

DataOrderingManager::Action
DataOrderingManager::receive(uint32_t opcode, DataDescriptor& desc, std::string data)
{
  SenderProfile *sender = checkSender(desc);
  sender->t = time(0) + 3600;

  if (sender->expectedId == 0)
  { // The first one, accept all
    sender->expectedId = calcExpectedId(desc.id);
    return REL;
  }

  if (desc.id == sender->expectedId)
  {
    sender->expectedId = calcExpectedId(desc.id);
    TempPacket * pkt = 0;
    if (sender->missingQueue.size() > 0 && ((pkt = sender->missingQueue.get(desc.id)) != 0))
    {
      sender->missingQueue.remove(pkt); 
    }

    if (sender->inQueue.size() > 0)
    {
      // Act#1
      DataBus::GetInstance()->getDataDecoder()->setReplay();
      DataBus::GetInstance()->getDataDecoder()->dispatch(opcode, desc, data);
      DataBus::GetInstance()->getDataDecoder()->clrReplay();

      uint32_t rFrom = sender->expectedId;
      bool found = true;
      do 
      {
        InputPacket * pkt = dynamic_cast<InputPacket*>(sender->inQueue.get(rFrom));
        if (!pkt)
        {
          found = false;
        }
        else
        {
          DataBus::GetInstance()->getDataDecoder()->setReplay();
          DataBus::GetInstance()->getDataDecoder()->dispatch(pkt->opcode, pkt->desc, pkt->data);
          DataBus::GetInstance()->getDataDecoder()->clrReplay();
          sender->inQueue.remove(pkt);

          rFrom = sender->expectedId + 1;
          if (rFrom > 0xFFFFFFF0)
          {
            rFrom = 1;
          }
        }
      }while (found);
      sender->expectedId = rFrom;

      if (sender->inQueue.size() == 0 && sender->missingQueue.size() > 0)
      {
        sender->missingQueue.clear();
      }
      return ACT1;
    }
    return REL;
  }
  else
  { // The incoming is not what I expected
    if (desc.id < sender->expectedId)
    {
      // Throw it away
      // Should I reset the expectedId to zero?
      return DIS;
    }
    else
    {
      // SAVE it
      std::string name;
      desc.from.toString(name, Address::RAW_MSG);
      sender->addMissingList(name, sender->expectedId, desc.id - 1);

      InputPacket * packet = new InputPacket();
      packet->opcode  = opcode;
      DataDescriptor::Copy(packet->desc, desc);
      packet->data.append(data.data(), data.size());
      packet->pktId   = desc.id;
      packet->t       = time(0) + 3600;
      sender->inQueue.add(desc.id, packet);
      return SAV;
    }
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
  if (list->size() == 0) return;

  OutputPacket * packet = dynamic_cast<OutputPacket*>(list->front());
  list->erase(list->begin());
  manager->send(packet->to, packet->opcode, packet->data, packet->pktId);
  packet->pending = 0;
  if (list->size() > 0)
  {
    schedule(0, 1);
  }
}

void 
DataOrderingManager::send(std::string to, uint32_t opcode, std::string data, uint32_t pktId)
{
  DataBus::GetInstance()->send(to, opcode, data, pktId);
}

SenderProfile::SenderProfile():
  t(0),
  expectedId(0)
{
}

SenderProfile::~SenderProfile()
{
}
