//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <boost/asio.hpp>
#include <net2/Nic.h>
#include <net2/NicList.h>
#include <net2/Resolver.h>
#include <net2/Service.h>
#include "Net.h"

namespace sai { 
namespace net2 {

class NetInfo
{
public:
  boost::asio::io_service io;
};

}}

using namespace sai::net2;

Net * Net::_Instance = 0;

Net::Net():
  _nicList(0),
  _resolver(0),
  _info(0)
{
  _nicList = new NicList();
  _resolver = new Resolver();
  _info = new NetInfo();

  _nicList->detect();
}

Net * 
Net::GetInstance()
{
  if (_Instance == 0)
  {
    _Instance = new Net();
  }
  return _Instance;
}

Net::~Net()
{
  while (_serviceList.size() > 0)
  {
    Service * svc = _serviceList.front();
    _serviceList.erase(_serviceList.begin());
    delete svc;
  }

  delete _info;
  delete _resolver;
  delete _nicList;
}

void 
Net::initialize()
{
  if (_serviceList.size() > 0)
  {
    ServiceListIterator iter;
    for (iter  = _serviceList.begin();
         iter != _serviceList.end();
         iter ++)
    {
      Service * svc = *iter;
      svc->deactivate();
      svc->activate();
    }
  }
}

void  
Net::mainLoop()
{
  _info->io.run();
}

void  
Net::shutdown()
{
  if (_serviceList.size() > 0)
  {
    ServiceListIterator iter;
    for (iter  = _serviceList.begin();
         iter != _serviceList.end();
         iter ++)
    {
      Service * svc = *iter;
      svc->deactivate();
    }
  }
  _info->io.stop();
}

void* 
Net::getIO()
{
  return ((void*) &(_info->io));
}

void  
Net::addService(Service * svc)
{
  _serviceList.push_back(svc);
}

