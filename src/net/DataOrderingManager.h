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

class SenderProfile : public TimerTask
{
public:
  typedef struct 
  {
    SenderProfile * profile;

    uint32_t     expectedId;
    uint32_t     name;
    DataQueue    inQueue;
    DataQueue    missingQueue;

    void addMissingList(std::string name, uint32_t from, uint32_t to);
    void releaseMessage();
  }Group;
  typedef std::map<uint32_t, Group*>           GroupTable;
  typedef std::map<uint32_t, Group*>::iterator GroupTableIterator;

  time_t       t;
  GroupTable   table;
  std::string  sender;

public:
  SenderProfile();
  ~SenderProfile();

  void timerEvent();
};

typedef std::map<std::string, SenderProfile*>           SenderTable;
typedef std::map<std::string, SenderProfile*>::iterator SenderTableIterator;

class DataOrderingManager : public TimerTask
{
public:
  typedef enum
  {
    REL   = 0,
    SAV   = 1,
    ACT1  = 2,
    DIS   = 3
  }Action;

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
  DataQueue                   _outQueue;
  PacketList                  _outgoingList;
  MsgRepeater                 _repeater;
  SenderTable                 _senderTable;

private:
  DataOrderingManager();
  void request(uint32_t id, std::string from);

  SenderProfile * checkSender(DataDescriptor& desc);
  inline uint32_t calcExpectedId(uint32_t cur);

public:
  static DataOrderingManager* GetInstance();
  ~DataOrderingManager();

  void initialize();
  void timerEvent(); 

  void processReqtEvent(DataDescriptor& desc, std::string& data);

  void save(DataDescriptor& desc, const char * data, uint32_t sz);
  void send(std::string to, uint32_t opcode, std::string data, int32_t pktId, int32_t grpId);

  Action receive(DataDescriptor& desc, std::string data);
};

inline
uint32_t 
DataOrderingManager::calcExpectedId(uint32_t cur)
{
  cur += 1;
  if (cur > 0xFFFFFFF0)
  {
    cur = 0;
  }
  return cur;
}

}
}

#endif
