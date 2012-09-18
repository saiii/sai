//=============================================================================
// Copyright (C) 2009 Athip Rooprayochsilp <athipr@gmail.com>
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

#ifndef __SAI_NET_DATABUS__
#define __SAI_NET_DATABUS__

#include <stdint.h>
#include <string>
#include <vector>
#include <net/ChainFilter.h>
#include <net/ProtocolDecoder.h>
#include <net/DataBusChannel.h>
#include <net/DataBusChannelImpl.h>
#include <net/PGMSocket.h>
#include <net/Net.h>

namespace sai 
{ 
namespace net 
{

class DataBusImpl;
class DataBusStateDb;
class DataBus : public ProtocolDecoder
{
friend class SenderProfile;
friend class DataOrderingManager;
private:
  static DataBus*  _instance;
  static DataBus** _instanceList;
  DataBusStateDb*  _stateDb;
  ChainFilter   *  _sendReceiveFilter;
  DataBusChannel*  _channel;

private:
  DataBus();
  bool send(std::string name, uint32_t id, DataDescriptor& desc, std::string data);

public:
  static DataBus * GetInstance();
  static DataBus * GetInstance(uint32_t index);
  ~DataBus();
  void setChannel(DataBusChannel*);

  void listen(std::string name);
  void activate();
  void deactivate();
  bool send(std::string name, uint32_t id, std::string data);
  bool send(PGMSocket*, std::string name, uint32_t id, std::string data);
  void blockSender(std::string name);

  DataBusChannel * getChannel();
  DataBusStateDb * getDb() { return _stateDb; }
};

class DataBusFilter : public ChainFilter
{
public:
  virtual bool filterEvent(DataDescriptor&, std::string&) = 0;
  virtual void block(std::string) = 0;
  virtual void block(uint32_t) = 0;
  virtual void add(std::string) = 0;
  virtual void add(uint32_t) = 0;
  virtual void clear() = 0;
};

}
}
#endif
