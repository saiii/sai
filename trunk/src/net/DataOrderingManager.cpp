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

bool 
DataOrderingManager::saveSeqNo(DataDescriptor& desc, std::string& data)
{
  enum { STOP_HERE = false, CONTINUE = true};

  SenderProfile *sender = checkSender(desc);
  _currentSender = sender;

  MissingPacket * mPkt = dynamic_cast<MissingPacket *>(_currentSender->missingQueue.get(desc.seqNo));
  if (mPkt)
  {
    _currentSender->missingQueue.remove(mPkt);
  }

  InputPacket * pkt = dynamic_cast<InputPacket*>(_currentSender->inQueue.get(desc.seqNo));
  if (pkt)
  { // duplicated message
    return STOP_HERE;
  }

  pkt = new InputPacket();
  pkt->desc = desc;
  pkt->t    = time(0) + 360;
  _currentSender->inQueue.add(pkt);

  return CONTINUE;
}

void 
DataOrderingManager::saveIncoming(DataDescriptor& desc, std::string& data)
{
  InputPacket * pkt = dynamic_cast<InputPacket*>(_currentSender->inQueue.get(desc.seqNo));
  pkt->data = data;
  pkt->desc = desc;

  char buf[17] = {0};
  memcpy(buf, desc.sender, 16);
  _currentSender->addMissingList(buf, _currentSender->expectedId, desc.seqNo - 1);
}

bool 
DataOrderingManager::isValid(DataDescriptor& desc, std::string& data)
{
  if (_currentSender->expectedId == desc.seqNo || _currentSender->expectedId == 0)
  {
    _currentSender->expectedId = desc.seqNo + 1;
    if (_currentSender->expectedId > 0xFFFFFFF0)
    {
      _currentSender->expectedId = 0;
    }
    return true;
  }

  return false;
}

void 
DataOrderingManager::releaseMessage(DataDescriptor& desc, std::string& data)
{
  _currentSender->releaseMessage();
}

void 
DataOrderingManager::saveOutgoing(DataDescriptor& desc, std::string& data, bool p2p)
{
  while (_outQueue.size() > 1000000) 
  {
    for (uint32_t i = 0; i < 1000; i += 1)
    {
      _outQueue.remove(0);
    }
  }

  OutputPacket * packet = new OutputPacket();
  packet->t       = time(0) + 360;
  packet->desc    = desc;
  packet->data    = data;
  packet->reqs    = 0;
  packet->pending = 0;
  packet->p2p     = p2p;

  _outQueue.add(packet);
}

SenderProfile * 
DataOrderingManager::checkSender(DataDescriptor& desc)
{
  SenderProfile * ret = 0;
  char tSender[17] = {0};
  memcpy(tSender, desc.sender, 16);
  SenderTableIterator iter = _senderTable.find(tSender);
  if (iter == _senderTable.end())
  {
    SenderProfile * sender = new SenderProfile(); 
    sender->sender     = tSender;
    sender->t          = time(0) + 360;
    sender->expectedId = 0;
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
      packet->t          = time(0);
      packet->desc.seqNo = v;
      packet->desc.to.str= name;
      packet->reqs       = 0;
      missingQueue.add(packet);
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

#if 0
  if (packet->t >= time(0)) 
  {
    schedule(0, 500);
    return;
  }
#endif

  char buff[32];
  sprintf(buff, "%u", packet->desc.seqNo);
  // Re-transmission request is always a point-to-point message

  // TODO : Enhance send function to support integer destination
  std::string to;
  packet->desc.to.toString(to, Address::RAW_MSG); 
  DataBus::GetInstance()->sendPointToPoint(to, SAI_REQ_OPCODE, buff, 0);
  packet->t = time(0) + 1;

  if (++packet->reqs > 10)
  {
    expectedId = packet->desc.seqNo + 1;
    if (expectedId > 0xFFFFFFF0)
    {
      expectedId = 1;
    }
    releaseMessage();
    missingQueue.remove(packet);
  }

  if (missingQueue.size() > 0)
  {
    schedule(0, 500);
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
      DataBus::GetInstance()->getDataDecoder()->dispatch(pkt->desc.opcode, pkt->desc, pkt->data);
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

  std::string to;
  packet->desc.to.toString(to, Address::RAW_MSG); 
  manager->send(to, packet->desc.opcode, packet->data, packet->desc.seqNo);
  packet->pending = 0;
  delete packet;
  if (list->size() > 0)
  {
    schedule(0, 1);
  }
}

void 
DataOrderingManager::send(std::string to, uint32_t opcode, std::string data, uint32_t seqNo)
{
  DataBus::GetInstance()->sendPointToPoint(to, opcode, data, seqNo);
}

SenderProfile::SenderProfile():
  expectedId(0),
  t(0)
{
}

SenderProfile::~SenderProfile()
{
}
