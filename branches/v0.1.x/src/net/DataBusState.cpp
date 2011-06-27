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
#include <time.h>
#include <windows.h>
#else
#include <syslog.h>
#endif
#include <cstdio>
#include <sstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <math/Utils.h>
#include "DataBusState.h"
#include "ProtocolEncoder.h"
#include "DataOrderingManager.h"
#include "Exception.h"

using namespace sai::net;

#define DROP_TEST 0

#ifdef DROP_TEST
uint32_t drop = 8;
#endif

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
 
  std::string localErrMsg = "";
  std::stringstream txt;
  try 
  {
    // Create server socket
    txt << "Unable to create server socket";
    localErrMsg = txt.str();
    next->_serverSocket = ServerSocket::Create(_db->_net,
                           SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_UDP,
                           SAI_SOCKET_B_REUSE_ADDR, SAI_SOCKET_OPT_TRUE,
                           SAI_SOCKET_B_USEIP_MULTICAST, SAI_SOCKET_OPT_TRUE,
                           SAI_SOCKET_EOA);
    next->_serverSocket->setEventHandler(next);
    McastDataBusChannel * channel = 0;
    channel = dynamic_cast<McastDataBusChannel*>(_db->_bus->getChannel());
    txt.str("");
    txt << "Unable to bind server socket to the specified port and address ";
    txt << "(" << channel->getLocalAddress() << "," << channel->getPort() << ")";
    localErrMsg = txt.str();
    next->_serverSocket->bind(channel->getLocalAddress(), channel->getPort());
    next->_serverSocket->open();

    txt.str("");
    txt << "Unable to join multicast group";
    localErrMsg = txt.str();
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

    txt.str("");
    txt << "Unable to create client socket";
    localErrMsg = txt.str();
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
      txt.str("");
      txt << "Unable to create direct server socket";
      localErrMsg = txt.str();
      next->_directServerSocket = ServerSocket::Create(_db->_net,
                                    SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_TCP,
                                    SAI_SOCKET_B_REUSE_ADDR, SAI_SOCKET_OPT_TRUE,
                                    SAI_SOCKET_EOA);
      next->_directServerSocket->bind(channel->getLocalAddress(), channel->getDirectPort());
      next->_directServerSocket->setEventHandler(next);
      next->_directServerSocket->open();
      next->_directServerSocket->listen();
    }

    _db->_state = next;
  } 
  catch (SocketException se)
  {
#ifdef _WIN32
    std::cerr << localErrMsg << std::endl;
    std::cerr << se.what() << std::endl;
#else
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "%s\n", localErrMsg.c_str());
    syslog(LOG_ERR, "%s\n", se.what());
    closelog();
#endif
    next->deactivate();
  }
  catch (...)
  {
#ifdef _WIN32
    std::cerr << localErrMsg << std::endl;
#else
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "%s\n", localErrMsg.c_str());
    closelog();
#endif
    next->deactivate();
  }
}

bool
NilMcastDataBusState::send(std::string name, uint32_t id, std::string data, int32_t seqNo)
{
  if (Net::GetInstance()->getLocalAddress().compare("127.0.0.1") == 0 ||
      Net::GetInstance()->getLocalAddress().compare("0.0.0.0") == 0)
  {
    Net::GetInstance()->initialize();
  }

  activate();
  if (_db->_state != this)
  {
    return _db->getState()->send(name, id, data, seqNo);
  }
  else
  {
    return false;
  }
}

bool 
NilMcastDataBusState::sendPointToPoint(std::string destination, 
                                       uint32_t    id, 
                                       std::string data, 
                                       int32_t     seqNo)
{
  if (Net::GetInstance()->getLocalAddress().compare("127.0.0.1") == 0 ||
      Net::GetInstance()->getLocalAddress().compare("0.0.0.0") == 0)
  {
    Net::GetInstance()->initialize();
  }

  activate();
  if (_db->_state != this)
  {
    return _db->getState()->sendPointToPoint(destination, id, data, seqNo);
  }
  else
  {
    return false;
  }
}

void 
NilMcastDataBusState::deactivate() 
{
  _db->_filter->clear();
  _db->_filter->add(Net::GetInstance()->getLocalAddressUInt32());
  _db->_filter->add(Net::GetInstance()->getLocalAddress());
  _db->_filter->add(Net::GetInstance()->getSenderId());
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

bool
ActiveMcastDataBusState::send(std::string name, uint32_t opcode, std::string data, int32_t seqNo) 
{
  Net * net = Net::GetInstance();

  sai::net::DataDescriptor desc;
  desc.version   = 1;
  memcpy(desc.sender, net->getSenderId(), sizeof(desc.sender));
  desc.seqNo     = seqNo == 0 ? net->getMessageId() : seqNo;
  desc.from.ival = net->getLocalAddressUInt32();
  desc.to.str    = name;
  desc.opcode    = opcode;

  if (seqNo == 0)
  {
    DataOrderingManager::GetInstance()->saveOutgoing(desc, data, false);
  }

#ifdef DROP_TEST
  if (desc.seqNo == drop)
  {
    // Find next drop
	sai::math::Utils::RandomSeed((sai::math::matrixsize_t)time(0));
    uint32_t next = sai::math::Utils::RandomInt(1, 30);
    drop = desc.seqNo + next;
    //printf("BLOCKED n NextDrop is %u\n", drop);
    return false;
  }
  //printf("Curr %u : DropPub %u\n", desc.seqNo, drop);
#endif

  std::string wireData;
  sai::net::ProtocolEncoder().encode(desc, opcode, data, wireData); 
  _clientSocket->send(wireData.data(), wireData.size());
  return true;
}

bool
ActiveMcastDataBusState::sendPointToPoint(std::string name, 
                                          uint32_t    opcode, 
                                          std::string data, 
                                          int32_t     seqNo)
{
  Net * net = Net::GetInstance();

  sai::net::DataDescriptor desc;
  desc.version   = 1;
  memcpy(desc.sender, net->getSenderId(), sizeof(desc.sender));
  desc.seqNo     = seqNo == 0 ? net->getMessageId() : seqNo;
  desc.from.ival = net->getLocalAddressUInt32();
  desc.to.str    = name;
  desc.opcode    = opcode;

  if (seqNo == 0)
  {
    DataOrderingManager::GetInstance()->saveOutgoing(desc, data, true);
  }

#ifdef DROP_TEST
  if (desc.seqNo == drop)
  {
    // Find next drop
    uint32_t next = sai::math::Utils::RandomInt(1, 30);
    drop = desc.seqNo + next;
    //printf("BLOCKED n NextDrop is %u\n", drop);
    return false;
  }
  //printf("Curr %u : DropPrv %u\n", desc.seqNo, drop);
#endif

  std::string wireData;
  sai::net::ProtocolEncoder().encode(desc, opcode, data, wireData); 
  _clientSocket->send(wireData.data(), wireData.size());
  return true;
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

