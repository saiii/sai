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
  uint32_t     expectedId;
  DataQueue    inQueue;
  DataQueue    missingQueue;

  time_t       t;
  std::string  sender;

public:
  SenderProfile();
  ~SenderProfile();

  void addMissingList(std::string name, uint32_t from, uint32_t to);
  void releaseMessage();

  void timerEvent();
};

typedef std::map<std::string, SenderProfile*>           SenderTable;
typedef std::map<std::string, SenderProfile*>::iterator SenderTableIterator;

class DataOrderingManager : public TimerTask,
                            public DataChecker
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
  DataQueue                   _outQueue;
  PacketList                  _outgoingList;
  MsgRepeater                 _repeater;
  SenderTable                 _senderTable;
  SenderProfile*              _currentSender;

private:
  DataOrderingManager();
  void request(uint32_t id, Address from);

  SenderProfile * checkSender(DataDescriptor& desc);
  void            send(std::string to, uint32_t opcode, std::string data, uint32_t seqNo);
  inline uint32_t calcExpectedId(uint32_t cur);

public:
  static DataOrderingManager* GetInstance();
  ~DataOrderingManager();

  void initialize();
  void timerEvent(); 

  void processReqtEvent(DataDescriptor& desc, std::string& data);

  void saveOutgoing(DataDescriptor& desc, std::string& data, bool p2p);

  bool saveSeqNo(DataDescriptor&, std::string&);
  void saveIncoming(DataDescriptor&, std::string&);
  bool isValid(DataDescriptor&, std::string&);
  void releaseMessage(DataDescriptor&, std::string&);
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
