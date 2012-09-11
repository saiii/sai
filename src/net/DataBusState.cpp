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

#ifndef _WIN32
#include <syslog.h>
#endif
#include <sstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "DataBusState.h"
#include "ProtocolEncoder.h"
//#include "DataOrderingManager.h"
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
 
  std::string localErrMsg = "";
  std::stringstream txt;
  try 
  {
    // Create server socket
    txt << "Unable to create server socket";
    localErrMsg = txt.str();
    McastDataBusChannel * channel = 0;
    channel = dynamic_cast<McastDataBusChannel*>(_db->_bus->getChannel());
    txt.str("");
    txt << "Unable to bind server socket to the specified port and address ";
    txt << "(" << channel->getLocalAddress() << "," << channel->getPort() << ")";
    localErrMsg = txt.str();

    txt.str("");
    txt << "Unable to join multicast group";
    localErrMsg = txt.str();
    StringList list;
    channel->getRecvMcast(list);

    NetworkOptions options;
    while (list.size() > 0)
    {
      std::string * str = list.front();
      list.erase(list.begin());
      options.addReceive(*str);
      delete str;
    }
	options.setSend(channel->getSendMcast());
    
    options.setInterface(channel->getLocalAddress());
    options.setPort(channel->getPort());

    next->_socket = new PGMSocket(&options);
    next->_socket->setEventHandler(next);
    next->_socket->listen();

    txt.str("");
    txt << "Unable to create client socket";
    localErrMsg = txt.str();

    _db->_state = next;
  } 
  catch (SocketException se)
  {
#ifdef _WIN32
    std::cerr << localErrMsg << std::endl;
    std::cerr << se.what() << std::endl;
#else
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, localErrMsg.c_str());
    syslog(LOG_ERR, se.what());
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
    syslog(LOG_ERR, localErrMsg.c_str());
    closelog();
#endif
    next->deactivate();
  }
}

bool
NilMcastDataBusState::send(std::string name, uint32_t id, std::string data)
{
  if (Net::GetInstance()->getLocalAddress().compare("127.0.0.1") == 0 ||
      Net::GetInstance()->getLocalAddress().compare("0.0.0.0") == 0)
  {
    Net::GetInstance()->initialize();
  }

  activate();
  if (_db->_state != this)
  {
    return _db->getState()->send(name, id, data);
  }
  else
  {
    return false;
  }
}

bool
NilMcastDataBusState::send(std::string name, uint32_t id, DataDescriptor&, std::string data)
{
  if (Net::GetInstance()->getLocalAddress().compare("127.0.0.1") == 0 ||
      Net::GetInstance()->getLocalAddress().compare("0.0.0.0") == 0)
  {
    Net::GetInstance()->initialize();
  }

  activate();
  if (_db->_state != this)
  {
    return _db->getState()->send(name, id, data);
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
  _db->_filter->add("*"); // For broadcast
}

ActiveMcastDataBusState::ActiveMcastDataBusState(DataBusStateDb * db, ProtocolDecoder * dec) :
  DataBusState(db),
  _decoder(dec)
{}

ActiveMcastDataBusState::~ActiveMcastDataBusState() 
{
  if (_socket)
  {
    _socket->close();
    delete _socket;
    _socket = 0;
  }
}

bool
ActiveMcastDataBusState::send(std::string name, uint32_t id, std::string data) 
{
  sai::net::DataDescriptor desc;
  desc.version   = 1;
  memcpy(desc.sender, Net::GetInstance()->getSenderId(), sizeof(desc.sender));
  desc.id        = Net::GetInstance()->getMessageId();
  desc.from.ival = Net::GetInstance()->getLocalAddressUInt32();
  desc.to.str    = name;

  //DataOrderingManager * mgr = DataOrderingManager::GetInstance();
  //mgr->addOutgoingData(desc, data);

  std::string wireData;
  sai::net::ProtocolEncoder().encode(desc, id, data, wireData); 
  _socket->send(wireData.data(), wireData.size());
  return true;
}

bool
ActiveMcastDataBusState::send(std::string name, uint32_t id, DataDescriptor& desc, std::string data) 
{
  std::string wireData;
  sai::net::ProtocolEncoder().encode(desc, id, data, wireData); 
  _socket->send(wireData.data(), wireData.size());
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
  if (_socket) _socket->close();
  delete _socket;
  _socket = 0;

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

