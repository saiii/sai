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

#ifndef __SAI_NET_DATAORDERINGMANAGER__
#define __SAI_NET_DATAORDERINGMANAGER__

#include <stdint.h>
#include <map>
#include <vector>
#include <string>
#include <net/DataDescriptor.h>
#include <net/DataDispatchable.h>
#include <net/DataHandler.h>
#include <net/TimerTask.h>
#include <net/DataOrderingQueue.h>

namespace sai
{
namespace net
{

typedef std::vector<uint32_t>           MissingList;
typedef std::vector<uint32_t>::iterator MissingListIterator;

class SenderProfile
{
public:
  time_t       t;
  uint32_t     expectedId;
  uint32_t     name;
  DataQueue    inQueue;
  MissingList  missingList;

public:
  SenderProfile();
  ~SenderProfile();
};

typedef std::map<uint32_t, SenderProfile*>           SenderTable;
typedef std::map<uint32_t, SenderProfile*>::iterator SenderTableIterator;

class DataOrderingManager : public TimerTask
{
private:
  class MsgRepeater : public TimerTask
  {
    public:
      PacketList          * list;
      DataOrderingManager * manager;

    public:
      void timerEvent();
  };

private:
  static DataOrderingManager* _instance;
  DataHandler *               _req;
  DataHandler *               _resp;
  DataQueue                   _outQueue;
  PacketList                  _outgoingList;
  MsgRepeater                 _repeater;
  SenderTable                 _senderTable;

private:
  DataOrderingManager();
  void request(uint32_t id, std::string from);

public:
  static DataOrderingManager* GetInstance();
  ~DataOrderingManager();

  void initialize();
  void timerEvent(); 

  void processReqtEvent(DataDescriptor& desc, std::string& data);
  void processRespEvent(DataDescriptor& desc, std::string& data);

  void save(uint32_t id, const char * data, uint32_t sz);
  void send(std::string to, uint32_t id, std::string data, bool save);

  bool receive(uint32_t from, uint32_t opcode, DataDescriptor& desc, std::string data);
};

}
}

#endif
