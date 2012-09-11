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
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <cstring>
#include <net/DataBusChannel.h>
#include <net/DataBusChannelImpl.h>

using namespace sai::net;

DataBusChannel::DataBusChannel():
  _localAddressUInt32(0),
  _directPort(0)
{}

DataBusChannel::~DataBusChannel()
{}

void 
DataBusChannel::setLocalAddress(std::string ip)
{
  _localAddress       = ip;
  _localAddressUInt32 = 0;
}

void 
DataBusChannel::setDirectPort(uint16_t port)
{
  _directPort = port;
}

uint16_t 
DataBusChannel::getDirectPort()
{
  return _directPort;
}

std::string 
DataBusChannel::getLocalAddress()
{
  static std::string empty = "";
  if (strcmp(_localAddress.c_str(), "0.0.0.0") == 0)
    return empty;
  else
    return _localAddress;
}

uint32_t 
DataBusChannel::getLocalAddressUInt32()
{
  if (_localAddressUInt32 == 0)
  {
    
#ifdef _WIN32
	extern uint32_t inetPton(std::string ip);
	_localAddressUInt32 = inetPton(_localAddress);
#else
	struct in_addr addr;
    inet_pton(AF_INET, _localAddress.c_str(), &addr);
	_localAddressUInt32 = htonl(addr.s_addr);
#endif
    
    return _localAddressUInt32;
  }
  else
  {
    return _localAddressUInt32;
  }
}

McastDataBusChannel::McastDataBusChannel():
  _impl(0)
{
  _impl = new McastDataBusChannelImpl;
  _impl->port = 0;
  _impl->sendMcast = "";
  _impl->recvMcastList.clear();
}

void 
McastDataBusChannel::copyFrom(DataBusChannel* o)
{
  McastDataBusChannel * other = dynamic_cast<McastDataBusChannel*>(o);
  if (!other) return;

  _impl->port     = other->_impl->port;
  _impl->sendMcast= other->_impl->sendMcast;

  _directPort         = other->_directPort;
  _localAddressUInt32 = other->_localAddressUInt32;
  _localAddress       = other->_localAddress;

  if (other->_impl->recvMcastList.size() > 0)
  {
    StringListIterator iter;
    for (iter  = other->_impl->recvMcastList.begin();
         iter != other->_impl->recvMcastList.end();
         iter ++)
    {
      std::string *str = new std::string(**iter);
      _impl->recvMcastList.push_back(str);
    }
  }
}

McastDataBusChannel::~McastDataBusChannel()
{
  delete _impl;
}

void
McastDataBusChannel::setPort(uint16_t port)
{
  _impl->port = port;
}

bool IsMcastAddress(std::string mcast)
{
  uint32_t a = 0;
#ifdef _WIN32
  extern uint32_t inetPton(std::string ip);
  a = inetPton(mcast);
#else
  struct in_addr addr;
  inet_pton(AF_INET, mcast.c_str(), &addr);
  a = htonl(addr.s_addr);
#endif
  a >>= 24;
 
  if (a >= 224 && a <= 239)
    return true;
  else
    return false;
}

void
McastDataBusChannel::setSendMcast(std::string mcast)
{
  if (IsMcastAddress(mcast))
    _impl->sendMcast = mcast;
}

void
McastDataBusChannel::addRecvMcast(std::string mcast)
{
  if (!IsMcastAddress(mcast))
    return;

  if (_impl->recvMcastList.size() > 0)
  {
    StringListIterator iter;
    for(iter  = _impl->recvMcastList.begin();
        iter != _impl->recvMcastList.end();
        iter ++)
    {
      std::string * str = *iter;
      if (str->compare(mcast) == 0)
      {
        return;
      }
    }
  }

  std::string * nstr = new std::string(mcast);
  _impl->recvMcastList.push_back(nstr);
}

uint16_t 
McastDataBusChannel::getPort()
{
  return _impl->port;
}

std::string 
McastDataBusChannel::getSendMcast()
{
  return _impl->sendMcast;
}

void 
McastDataBusChannel::getRecvMcast(StringList& ret)
{
  if (_impl->recvMcastList.size() > 0)
  {
    StringListIterator iter;
    for (iter  = _impl->recvMcastList.begin();
         iter != _impl->recvMcastList.end();
         iter ++)
    {
      std::string *str = new std::string(**iter);
      ret.push_back(str);
    }
  }
}

McastDataBusChannelImpl::~McastDataBusChannelImpl()
{
  std::string * str = 0;
  while (recvMcastList.size() > 0)
  {
    str = recvMcastList.front();
    recvMcastList.erase(recvMcastList.begin());
    delete str;
  }
}

