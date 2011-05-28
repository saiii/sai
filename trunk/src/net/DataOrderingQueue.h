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

#ifndef __SAI_NET_DATAORDERINGQUEUE__
#define __SAI_NET_DATAORDERINGQUEUE__

#include <stdint.h>
#include <map>
#include <vector>
#include <string>
#include <net/DataDescriptor.h>

namespace sai
{
namespace net
{

class TempPacket
{
public:
  time_t         t;
  DataDescriptor desc;
  std::string    data;

public:
  TempPacket();
  virtual ~TempPacket();
};

class OutputPacket : public TempPacket
{
public:
  uint16_t       reqs;
  OutputPacket * pending;
  bool           p2p;

public:
  OutputPacket();
  ~OutputPacket();
};

class InputPacket : public TempPacket
{
public:
  InputPacket();
  ~InputPacket();
};

class MissingPacket : public TempPacket
{
public:
  uint32_t    reqs;

public:
  MissingPacket();
  ~MissingPacket();
};

typedef std::map<uint32_t, TempPacket*>           PacketTable;
typedef std::map<uint32_t, TempPacket*>::iterator PacketTableIterator;
typedef std::vector<TempPacket*>                  PacketList;
typedef std::vector<TempPacket*>::iterator        PacketListIterator;

class DataQueue
{
private:
  PacketTable _table;
  PacketList  _list;

public:
  DataQueue();
  virtual ~DataQueue();

  void add(TempPacket*);
  void removeAt(uint32_t index);
  void remove(TempPacket* packet);
  TempPacket* popAndPutLast();
  TempPacket* at(uint32_t index);
  TempPacket* get(uint32_t id);
  uint32_t size();
  void clear();
};

}
}

#endif
