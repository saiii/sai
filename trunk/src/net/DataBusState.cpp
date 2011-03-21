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

#ifdef _WIN32
#include <inet_pton.h>
#endif
#include <iostream>
#include <algorithm>
#include "DataBusState.h"
#include "ProtocolEncoder.h"
#include "Exception.h"

using namespace sai::net;

DataBusStateDb::DataBusStateDb(Net& net, 
                               DataBus * bus, 
                               ProtocolDecoder* dec, 
                               DataBusFilter* fil):
  _net(net),
  _bus(bus),
  _decoder(dec),
  _filter(fil),
  _nilMcast(0),
  _activeMcast(0),
  _state(0)
{
  _nilMcast    = new NilMcastDataBusState(this);
  _activeMcast = new ActiveMcastDataBusState(this, _decoder);
  _state       = _nilMcast;
}

DataBusStateDb::~DataBusStateDb()
{
  delete _nilMcast;
  delete _activeMcast;
}

NilMcastDataBusState::NilMcastDataBusState(DataBusStateDb *db) : 
  DataBusState(db)
{
  deactivate();
}

NilMcastDataBusState::~NilMcastDataBusState() 
{}

void 
NilMcastDataBusState::listen(std::string name) 
{
  _db->_filter->add(name);
}

void 
NilMcastDataBusState::blockSender(std::string name) 
{
  _db->_filter->block(name);
}

void 
NilMcastDataBusState::activate() 
{
  ActiveMcastDataBusState* next = dynamic_cast<ActiveMcastDataBusState*>(
                                    _db->getActiveMcastState());
  if (!next) return;
 
  try 
  {
    // Create server socket
    next->_serverSocket = ServerSocket::Create(_db->_net,
                           SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_UDP,
                           SAI_SOCKET_B_REUSE_ADDR, SAI_SOCKET_OPT_TRUE,
                           SAI_SOCKET_B_USEIP_MULTICAST, SAI_SOCKET_OPT_TRUE,
                           SAI_SOCKET_EOA);
    next->_serverSocket->setEventHandler(next);
    McastDataBusChannel * channel = 0;
    channel = dynamic_cast<McastDataBusChannel*>(_db->_bus->getChannel());
    next->_serverSocket->bind(channel->getLocalAddress(), channel->getPort());
    next->_serverSocket->open();

    StringList list;
    channel->getRecvMcast(list);
    while (list.size() > 0)
    {
      std::string * str = list.front();
      list.erase(list.begin());
      next->_serverSocket->join(*str);
      delete str;
    }

    next->_serverSocket->listen();

    // Create client socket
    next->_clientSocket = ClientSocket::Create(_db->_net,
                            SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_UDP,
                            SAI_SOCKET_B_USEIP_MULTICAST, SAI_SOCKET_OPT_TRUE,
                            SAI_SOCKET_EOA);
    next->_clientSocket->open();
    next->_clientSocket->connect(channel->getSendMcast(), channel->getPort());


    // Create Direct Server Socket (if needed)
    if (channel->getDirectPort())
    {
      next->_directServerSocket = ServerSocket::Create(_db->_net,
                                    SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_TCP,
                                    SAI_SOCKET_B_REUSE_ADDR, SAI_SOCKET_OPT_TRUE,
                                    SAI_SOCKET_EOA);
      next->_directServerSocket->bind(channel->getLocalAddress(), channel->getDirectPort());
      next->_directServerSocket->setEventHandler(next);
      next->_directServerSocket->open();
      next->_directServerSocket->listen();
    }
  } 
  catch (SocketException se)
  {
    std::cerr << "Error" << std::endl;
    next->deactivate();
    //return;
  }
  catch (...)
  {
    std::cerr << "Error2" << std::endl;
  }
  _db->_state = next;
}

void 
NilMcastDataBusState::deactivate() 
{
  _db->_filter->clear();

  DataBusChannel * channel = _db->_bus->getChannel();
  std::string localAddress = channel->getLocalAddress();
  if (localAddress.length() > 0) // For point-to-point 
  {
    struct in_addr addr;
    inet_pton(AF_INET, localAddress.c_str(), &addr);
    unsigned long int a = htonl(addr.s_addr);
    if (a > 0)
    {
      _db->_filter->add(a);
      _db->_filter->add(localAddress);
    }
  }
  _db->_filter->add("*"); // For broadcast
}

ActiveMcastDataBusState::ActiveMcastDataBusState(DataBusStateDb * db, ProtocolDecoder * dec) :
  DataBusState(db),
  _decoder(dec),
  _serverSocket(0),
  _clientSocket(0),
  _directServerSocket(0)
{}

ActiveMcastDataBusState::~ActiveMcastDataBusState() 
{
  if (_serverSocket)
  {
    _serverSocket->close();
    delete _serverSocket;
    _serverSocket = 0;
  }

  if (_clientSocket)
  {
    _clientSocket->close();
    delete _clientSocket;
    _clientSocket = 0;
  }

  if (_directServerSocket)
  {
    _directServerSocket->close();
    delete _directServerSocket;
    _directServerSocket = 0;
  }
}

void 
ActiveMcastDataBusState::send(std::string name, uint32_t id, std::string data) 
{
  sai::net::DataDescriptor desc;
  desc.version   = 1;
  memcpy(desc.sender, Net::GetInstance()->getSenderId(), sizeof(desc.sender));
  desc.id        = Net::GetInstance()->getMessageId();
  desc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
  desc.to.str    = name;

  std::string wireData;
  sai::net::ProtocolEncoder().encode(desc, id, data, wireData); 
  _clientSocket->send(wireData.c_str(), wireData.size());
}

void 
ActiveMcastDataBusState::send(std::string name, uint32_t id, DataDescriptor& desc, std::string data) 
{
  std::string wireData;
  sai::net::ProtocolEncoder().encode(desc, id, data, wireData); 
  _clientSocket->send(wireData.c_str(), wireData.size());
}

void 
ActiveMcastDataBusState::blockSender(std::string name) 
{
  _db->_filter->block(name);
}

void 
ActiveMcastDataBusState::deactivate() 
{
  if (_serverSocket) _serverSocket->close();
  if (_clientSocket) _clientSocket->close();
  delete _serverSocket;
  delete _clientSocket;
  _serverSocket = 0;
  _clientSocket = 0;

  _db->_filter->clear();
  _db->_state = _db->getNilMcastState();
}

void 
ActiveMcastDataBusState::processDataEvent(char *data, uint32_t size)
{
  std::string dat;
  dat.append(data, size);
  DataDescriptor desc;
  _db->_decoder->processDataEvent(desc, dat);
}

bool 
ActiveMcastDataBusState::processConnectionEvent(std::string ip)
{
  DataDescriptor desc;
  std::string dummy;
  desc.from.str = ip;
  return _db->_filter->filterEvent(desc, dummy);
}

void 
ActiveMcastDataBusState::processConnectedEvent(ClientSocket * sckt)
{
}

