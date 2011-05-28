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
  _req(0),
  _currentSender(0)
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
  while (_outgoingList.size() > 0)
  {
    OutputPacket * packet = dynamic_cast<OutputPacket*>(_outgoingList.front());
    _outgoingList.erase(_outgoingList.begin());
    delete packet;
  }

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
  uint32_t seqNo = atoi(data.c_str());

  request(seqNo, desc.from);
}

void 
DataOrderingManager::save(DataDescriptor& desc, const char * data, uint32_t sz)
{
#if 0
  if (opcode == SAI_REQ_OPCODE)
  {
    return;
  }
#endif

  while (_outQueue.size() > 1000000) 
  {
    for (uint32_t i = 0; i < 1000; i += 1)
    {
      _outQueue.remove(0);
    }
  }

  OutputPacket * packet = new OutputPacket();
  packet->data.append(data, sz);
  packet->pktId   = desc.seqNo;
  packet->grpId   = desc.groupId;
  packet->opcode  = desc.opcode;
  packet->t       = time(0) + 300;
  packet->desc    = desc;
  packet->data    = data;
  packet->reqs    = 0;
  packet->pending = 0;
  packet->p2p     = pointToPoint;

  _outQueue.add(desc.seqNo, packet);
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
    _senderTable.insert(std::make_pair(sender->sender, sender));

    SenderProfile::Group * group = new SenderProfile::Group();
    group->profile    = sender;
    group->expectedId = 0;
    sender->table.insert(std::make_pair(desc.groupId, group));

    ret = sender;
  }
  else
  {
    ret    = iter->second;
    ret->t = time(0) + 360;
  }

  SenderProfile::GroupTableIterator iter2 = ret->table.find(desc.groupId);
  if (iter2 == ret->table.end())
  {
    SenderProfile::Group * grp = new SenderProfile::Group();
    grp->expectedId = 0;  
    ret->table.insert(std::make_pair(desc.groupId, grp));
  }

  return ret;
}

void 
SenderProfile::Group::addMissingList(std::string name, uint32_t from, uint32_t to)
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
      missingQueue.add(v, packet);
    }
  }

  if (emptyQueue && missingQueue.size() > 0)
  {
    profile->schedule(0, 1);
  }
}

void 
SenderProfile::timerEvent()
{
  bool doMore = false;
  GroupTableIterator iter;
  for (iter  = table.begin();
       iter != table.end();
       iter ++)
  {
    Group * group = iter->second;
    if (group->missingQueue.size() == 0)
    {
      continue;
    }

    MissingPacket * packet = dynamic_cast<MissingPacket*>(group->missingQueue.popAndPutLast());
    if (!packet)
    {
      continue;
    }

    if (packet->t >= time(0)) 
    {
      doMore = true;
      continue;
    }
#endif

    char buff[32];
    sprintf(buff, "%u", packet->pktId);
    // Re-transmission request is always a point-to-point message
    DataBus::GetInstance()->sendPointToPoint(packet->name, SAI_REQ_OPCODE, buff, 0, -1);
    packet->t = time(0) + 1;

    if (++packet->reqs > 10)
    {
      group->expectedId = packet->pktId + 1;
      if (group->expectedId > 0xFFFFFFF0)
      {
        group->expectedId = 1;
      }
      group->releaseMessage();
      group->missingQueue.remove(packet);
    }

    if (group->missingQueue.size() > 0)
    {
      doMore = true;
    }
  }

  if (doMore)
  {
    schedule(0, 500);
  }
}

void 
SenderProfile::Group::releaseMessage()
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
      DataBus::GetInstance()->getDataDecoder()->dispatch(pkt->desc.opcode, pkt->desc, pkt->data);
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
DataOrderingManager::receive(DataDescriptor& desc, std::string data)
{
  enum { CONTINUE = true, STOP_HERE = false };

  SenderProfile *sender = checkSender(desc);
  _currentSender = sender;

  SenderProfile::GroupTableIterator iter = sender->table.find(desc.groupId);
  SenderProfile::Group * group = iter->second;

  if (group->expectedId == 0)
  { // The first one, accept all
    group->expectedId = calcExpectedId(desc.seqNo);
    return REL;
  }

  if (desc.seqNo == group->expectedId)
  {
    group->expectedId = calcExpectedId(desc.seqNo);
    TempPacket * pkt = 0;
    if (group->missingQueue.size() > 0 && ((pkt = group->missingQueue.get(desc.seqNo)) != 0))
    {
      group->missingQueue.remove(pkt); 
    }

    if (group->inQueue.size() > 0)
    {
      // Act#1
      DataBus::GetInstance()->getDataDecoder()->setReplay();
      DataBus::GetInstance()->getDataDecoder()->dispatch(desc.opcode, desc, data);
      DataBus::GetInstance()->getDataDecoder()->clrReplay();

      uint32_t rFrom = group->expectedId;
      bool found = true;
      do 
      {
        InputPacket * pkt = dynamic_cast<InputPacket*>(group->inQueue.get(rFrom));
        if (!pkt)
        {
          found = false;
        }
        else
        {
          DataBus::GetInstance()->getDataDecoder()->setReplay();
          DataBus::GetInstance()->getDataDecoder()->dispatch(pkt->desc.opcode, pkt->desc, pkt->data);
          DataBus::GetInstance()->getDataDecoder()->clrReplay();
          group->inQueue.remove(pkt);

          rFrom += 1;
          if (rFrom > 0xFFFFFFF0)
          {
            rFrom = 1;
          }
        }
      }while (found);
      group->expectedId = rFrom;

      if (group->inQueue.size() == 0 && group->missingQueue.size() > 0)
      {
        group->missingQueue.clear();
      }
      return ACT1;
    }
    return REL;
  }
  else
  { // The incoming is not what I expected
    if (desc.seqNo < group->expectedId)
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
      group->addMissingList(name, group->expectedId, desc.seqNo - 1);

      InputPacket * packet = new InputPacket();
      DataDescriptor::Copy(packet->desc, desc);
      packet->data.append(data.data(), data.size());
      packet->pktId   = desc.seqNo;
      packet->t       = time(0) + 3600;
      group->inQueue.add(desc.seqNo, packet);
      return SAV;
    }
  }
}

void
DataOrderingManager::request(uint32_t seqNo, Address from)
{
  OutputPacket * pkt = dynamic_cast<OutputPacket*>(_outQueue.get(seqNo));
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

  OutputPacket * nPkt = new OutputPacket();
  nPkt->t       = time(0);
  nPkt->desc    = pkt->desc;
  nPkt->desc.to = from;
  nPkt->data    = pkt->data;
  nPkt->reqs    = pkt->reqs;
  nPkt->pending = pkt;
  nPkt->p2p     = pkt->p2p;

  pkt->pending  = nPkt;

  if (pkt->p2p)
  {
    std::string aTo, bTo;
    from.toString(aTo, Address::RAW_MSG);
    pkt->desc.to.toString(bTo, Address::RAW_MSG);
    if (aTo.compare(bTo) != 0)
    {
      nPkt->desc.opcode = 0;
      nPkt->data.clear();
    }
  }

  _outgoingList.push_back(nPkt);
}

void 
DataOrderingManager::MsgRepeater::timerEvent()
{
  if (list->size() == 0) return;

  OutputPacket * packet = dynamic_cast<OutputPacket*>(list->front());
  list->erase(list->begin());
  manager->send(packet->to, packet->opcode, packet->data, packet->pktId, packet->grpId);
  packet->pending = 0;
  if (list->size() > 0)
  {
    schedule(0, 1);
  }
}

void 
DataOrderingManager::send(std::string to, uint32_t opcode, std::string data, int32_t pktId, int32_t grpId)
{
  DataBus::GetInstance()->sendPointToPoint(to, opcode, data, pktId, grpId);
}

SenderProfile::SenderProfile():
  t(0)
{
}

SenderProfile::~SenderProfile()
{
  GroupTableIterator iter;
  while (table.size() > 0)
  {
    iter = table.begin();
    Group * group = iter->second;
    table.erase(iter);
    delete group;
  }
}
