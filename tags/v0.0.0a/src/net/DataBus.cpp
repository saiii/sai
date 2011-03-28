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
#include <inet_pton.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <boost/asio.hpp>
#include "Net.h"
#include "DataBus.h"
#include "DataBusState.h"
//#include "DataOrderingManager.h"
#include "Socket.h"

using namespace sai::net;

namespace sai 
{ 
namespace net 
{

class _SendReceiveFilter : public DataBusFilter
{
public:
  StringList _blockSenderListString;
  IntList    _blockSenderListInt;
  StringList _listenListString;
  IntList    _listenListInt;

public:
  ~_SendReceiveFilter();
  bool filterEvent(DataDescriptor&, std::string&);
  void block(std::string);
  void block(uint32_t);
  void add(std::string);
  void add(uint32_t);
  void clear();
};

}
}

DataBus* DataBus::_instance = 0;

DataBus::DataBus():
  _stateDb(0),
  _sendReceiveFilter(0)
{
  activateChecker();
}

DataBus::~DataBus()
{
  delete _channel;
  delete _sendReceiveFilter;
  delete _stateDb;
  //delete DataOrderingManager::GetInstance();
}

DataBus * DataBus::GetInstance()
{
  if (_instance == 0)
  {
    _instance = new DataBus();
  }
  return _instance;
}

void 
DataBus::setChannel(DataBusChannel* chnl)
{
  Net * net = Net::GetInstance();

  if (dynamic_cast<McastDataBusChannel*>(chnl))
  {
    _channel = new McastDataBusChannel();
  }
  _channel->copyFrom(chnl);

  _SendReceiveFilter * filter = new _SendReceiveFilter();
  _sendReceiveFilter = filter;

  _stateDb = new DataBusStateDb(*net, this, this, filter);
  _fromTo.setFilter(_sendReceiveFilter);

  if (_channel->_localAddress.length() == 0)
  {
    _channel->_localAddress = Net::GetInstance()->getLocalAddress();
    _channel->_localAddressUInt32 = 0;
  }
}

void 
DataBus::listen(std::string name)
{
  _stateDb->getState()->listen(name);
}

void 
DataBus::activate()
{
  //DataOrderingManager* order = new DataOrderingManager(&_data);
  //DataOrderingManager::_instance = order;
  //order->initialize();
  _stateDb->getState()->activate();
}

void 
DataBus::deactivate()
{
  _stateDb->getState()->deactivate();
}

void 
DataBus::send(std::string name, uint32_t id, std::string data)
{
  _stateDb->getState()->send(name, id, data);
}

void 
DataBus::send(std::string name, uint32_t id, DataDescriptor& desc, std::string data)
{
  _stateDb->getState()->send(name, id, desc, data);
}

void 
DataBus::blockSender(std::string name)
{
  _stateDb->getState()->blockSender(name);
}

DataBusChannel * 
DataBus::getChannel()
{
  return _channel;
}

inline bool ContainIn(IntList& list, DataDescriptor& desc, bool checkFrom)
{
  IntListIterator iter;
  for (iter  = list.begin();
       iter != list.end();
       iter ++)
  {
    uint32_t i = *iter;
    if (checkFrom)
    {
      if (i == desc.from.ival)
      {
        return true;
      }
    }
    else
    {
      if (i == desc.to.ival)
      {
        return true;
      }
    }
  }

  return false;
}

inline bool ContainIn(StringList& list, DataDescriptor& desc, bool checkFrom)
{
  StringListIterator siter;
  for (siter  = list.begin();
       siter != list.end();
       siter ++)
  {
    std::string *str = *siter;
    if (checkFrom)
    {
      if (str->compare(0, str->length(), desc.from.str) == 0)
      {
        return true;
      }
    }
    else
    {
      if (str->compare(0, str->length(), desc.to.str) == 0)
      {
        return true;
      }
    }
  }

  return false;
}

bool
_SendReceiveFilter::filterEvent(DataDescriptor& desc, std::string& data)
{
  // is desc.from in blocksender list?
  if (_blockSenderListInt.size() > 0 && ContainIn(_blockSenderListInt, desc, true)) 
  {
    return false;
  }

  if (_blockSenderListString.size() > 0 && ContainIn(_blockSenderListString, desc, true))
  {
    return false;
  }

  // is desc.to is ourself?
  if (desc.to.ival == 0)
  {
    if (_listenListString.size() > 0 && ContainIn(_listenListString, desc, false))
    {
      return true;
    }
  }
  else
  {
    if (_listenListInt.size() > 0 && ContainIn(_listenListInt, desc, false))
    {
      return true;
    }
  }

  return false;
}

void 
_SendReceiveFilter::block(std::string name)
{
  StringListIterator iter;
  for(iter  = _blockSenderListString.begin();
      iter != _blockSenderListString.end();
      iter ++)
  {
    std::string * str = *iter;
    if (str->compare(0, str->length(), name) == 0) 
    { 
      return; 
    }
  }

  _blockSenderListString.push_back(new std::string(name));
}

void 
_SendReceiveFilter::block(uint32_t addr)
{
  IntListIterator iter;
  iter = std::find(_blockSenderListInt.begin(), _blockSenderListInt.end(), addr);
  if (iter == _blockSenderListInt.end())
  {
    _blockSenderListInt.push_back(addr);
  }
}

void 
_SendReceiveFilter::add(std::string name)
{
  StringListIterator iter;
  for (iter  = _listenListString.begin();
       iter != _listenListString.end();
       iter ++)
  {
    std::string * str = *iter;
    if (str->compare(0, str->length(), name) == 0)
    {
      return;
    }
  }

  _listenListString.push_back(new std::string(name));
}

void 
_SendReceiveFilter::add(uint32_t addr)
{
  IntListIterator iter;
  iter = std::find(_listenListInt.begin(), _listenListInt.end(), addr);
  if (iter == _listenListInt.end())
  {
    _listenListInt.push_back(addr);
  }
}

void 
_SendReceiveFilter::clear()
{
  _blockSenderListString.clear();
  _blockSenderListInt.clear();
  _listenListString.clear();
  _listenListInt.clear();
}

_SendReceiveFilter::~_SendReceiveFilter()
{
  std::string * str = 0;
  while (_blockSenderListString.size() > 0)
  {
    str = _blockSenderListString.front();
    _blockSenderListString.erase(_blockSenderListString.begin());
    delete str;
  }

  while (_listenListString.size() > 0)
  {
    str = _listenListString.front();
    _listenListString.erase(_listenListString.begin());
    delete str;
  }
}

