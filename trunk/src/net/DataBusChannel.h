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

#ifndef __SAI_NET_DATABUSCHANNEL__
#define __SAI_NET_DATABUSCHANNEL__

#include <stdint.h>
#include <string>
#include <net/Net.h>

namespace sai 
{ 
namespace net 
{

class DataBus;
class DataBusChannel
{
friend class DataBus;
protected:
  std::string _localAddress;
  uint32_t    _localAddressUInt32;
  uint16_t    _directPort;
  bool        _classicChannel;
  bool        _enableHttpInterface;

public:
  DataBusChannel();
  virtual ~DataBusChannel();
  virtual void copyFrom(DataBusChannel* other) {}
  virtual void setLocalAddress(std::string ip);
  virtual void setClassicChannel(bool v);
  virtual void setEnableHttpInterface(bool v);

  virtual void        setDirectPort(uint16_t port);
  virtual uint16_t    getDirectPort();
  virtual std::string getLocalAddress();
  virtual uint32_t    getLocalAddressUInt32();
  virtual bool        isClassicChannel();
  virtual bool        enableHttpInterface();
};

class McastDataBusChannelImpl;
class McastDataBusChannel : public DataBusChannel
{
friend class DataBus;
private:
  McastDataBusChannelImpl * _impl; 

public:
  McastDataBusChannel();
  ~McastDataBusChannel();

  void copyFrom(DataBusChannel* other);
  void setPort(uint16_t port);
  void setSendMcast(std::string mcast);
  void addRecvMcast(std::string mcast);

  uint16_t    getPort();
  std::string getSendMcast();
  void        getRecvMcast(StringList&);
};

}
}
#endif
