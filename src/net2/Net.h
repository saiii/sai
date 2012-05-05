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

#ifndef __SAI_NET2_NET__
#define __SAI_NET2_NET__

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

namespace sai 
{ 
namespace net2
{

class Net;
class Nic
{
friend class Net;
private:
  std::string _name;
  std::string _ip;
  std::string _bcast;

private:
  Nic();

public:
  ~Nic();

  const char * name() { return _name.c_str(); }
  const char * ip()   { return _ip.c_str();   }
  const char * bcast(){ return _bcast.c_str();}
};

typedef std::vector<Nic*>           NicList;
typedef std::vector<Nic*>::iterator NicListIterator;

class NetImpl;
class Net
{
private:
  NetImpl*     _impl;
  NicList      _nicList;
  static Net * _instance;
  std::string  _hostAddress;
  std::string  _hostBcastAddress;
  uint32_t     _hostAddressUInt32;
  std::string  _preferredAddress;

private:
  Net();
  void getHostAddress();
  void sortNicList(NicList& list);

public:
  ~Net();
  static Net * GetInstance();
  void         setPreferredAddress(const char * addr) { _preferredAddress = addr; }

  void        initialize();
  std::string getIpFromName(std::string);
  std::string getLocalAddress() { return _hostAddress; }
  std::string getLocalBroadcastAddress() { return _hostBcastAddress; }
  uint32_t    getLocalAddressUInt32() { return _hostAddressUInt32; }
  std::string getLocalIpFromNic(std::string);
  uint32_t    getNumNic() { return _nicList.size(); }
  std::string getNicList(std::string& ret);

  void        mainLoop();
  void        shutdown();
  void*       getIO();
};

}
}
#endif
