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

#ifndef __SAI_NET_DATABUSSTATE__
#define __SAI_NET_DATABUSSTATE__

#include <string>
#include <net/Net.h>
#include <net/Socket.h>
#include <net/DataBus.h>

namespace sai 
{ 
namespace net 
{

class DataBusStateDb;
class DataBusState
{
protected:
  DataBusStateDb * _db;

public:
  DataBusState(DataBusStateDb *db):_db(db) {}
  virtual ~DataBusState() {}

  virtual void listen(std::string name) {}
  virtual void activate() {}
  virtual void deactivate() {}
  virtual bool send(std::string name, uint32_t id, std::string data, bool save) { return false; }
  virtual void blockSender(std::string name) {}
};

class NilMcastDataBusState : public DataBusState
{
public:
  NilMcastDataBusState(DataBusStateDb *);
  ~NilMcastDataBusState();

  void listen(std::string name);
  void blockSender(std::string name);
  void activate();
  void deactivate();
  bool send(std::string name, uint32_t id, std::string data, bool save);
};

class ActiveMcastDataBusState : public DataBusState,
                                public SocketEventHandler
{
friend class NilMcastDataBusState;
private:
  ProtocolDecoder * _decoder;
  ServerSocket    * _serverSocket;
  ClientSocket    * _clientSocket;
  ServerSocket    * _directServerSocket;

public:
  ActiveMcastDataBusState(DataBusStateDb*, ProtocolDecoder *);
  ~ActiveMcastDataBusState();

  bool send(std::string name, uint32_t id, std::string data, bool save);
  void blockSender(std::string name);
  void deactivate();

  void processDataEvent(char *data, uint32_t size);
  bool processConnectionEvent(std::string ip);
  void processConnectedEvent(ClientSocket * sckt);
};

class DataBusStateDb
{
friend class DataBusState;
friend class NilMcastDataBusState;
friend class ActiveMcastDataBusState;
private:
  Net&              _net;
  DataBus         * _bus;
  ProtocolDecoder * _decoder;
  DataBusFilter   * _filter;
  DataBusState    * _nilMcast;
  DataBusState    * _activeMcast;
  DataBusState    * _state;

public:
  DataBusStateDb(Net&, DataBus * bus, ProtocolDecoder* dec, DataBusFilter* fil);
  ~DataBusStateDb();

  DataBusState* getNilMcastState()     { return _nilMcast;    		}
  DataBusState* getActiveMcastState()  { return _activeMcast; 		}  
  DataBusState* getState()             { return _state;       		}
};

}
}

#endif
