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
#include <net/DataOrderingQueue.h>

using namespace sai::net;

DataQueue::DataQueue() 
{
}

DataQueue::~DataQueue() 
{
  PacketTableIterator iter;
  while (_table.size() > 0)
  {
    iter = _table.begin();
    TempPacket* packet = iter->second;
    _table.erase(iter);
    delete packet;
  }

  _list.clear();
}

void      
DataQueue::add(TempPacket* packet)
{
  if (_table.find(packet->desc.seqNo) == _table.end())
  {
    _list.push_back(packet);
    _table.insert(std::make_pair(packet->desc.seqNo, packet));
  }
}

void 
DataQueue::removeAt(uint32_t index)
{
  TempPacket* packet = _list.at(index);
  remove(packet);
}

void 
DataQueue::remove(TempPacket* packet)
{
  PacketTableIterator iter;
  if ((iter = _table.find(packet->desc.seqNo)) != _table.end())
  {
    TempPacket* pckt = iter->second;
    _table.erase(iter);

    PacketListIterator l = std::find(_list.begin(), _list.end(), pckt);
    _list.erase(l);
    delete pckt;
  }
}

TempPacket* 
DataQueue::popAndPutLast()
{
  if (_list.size() == 0) return 0;

  TempPacket* packet = _list.at(0);
  _list.erase(_list.begin());
  _list.push_back(packet);
  
  return packet;
}

TempPacket* 
DataQueue::at(uint32_t index)
{
  return _list.at(index);
}

TempPacket*
DataQueue::get(uint32_t id)
{
  PacketTableIterator iter;
  if ((iter = _table.find(id)) != _table.end())
  {
    TempPacket* ret = iter->second;
    return ret;
  }

  return 0;
}

uint32_t 
DataQueue::size()
{
  return _list.size();
}

void
DataQueue::clear()
{
  PacketTableIterator iter;
  while (_table.size() > 0)
  {
    iter = _table.begin();
    TempPacket* packet = iter->second;
    _table.erase(iter);
    delete packet;
  }

  _list.clear();
}

TempPacket::TempPacket():
  t(0)
{
}

TempPacket::~TempPacket()
{
}

OutputPacket::OutputPacket():
  reqs(0),
  p2p(false),
  pending(0)
{
}

OutputPacket::~OutputPacket()
{
}

InputPacket::InputPacket()
{
}

InputPacket::~InputPacket()
{
}

MissingPacket::MissingPacket()
{
}

MissingPacket::~MissingPacket()
{
}

