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
#include <Ws2tcpip.h>
#endif
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <utils/Logger.h>
#include "Net.h"

namespace sai { namespace net {
class NetImpl
{
public:
  boost::asio::io_service io;

public:
  NetImpl() {}
  ~NetImpl(){}

  boost::asio::io_service* getIO() { return &io; }
};
}}

using namespace sai::net;

Net * Net::_instance = 0;

Net::Net():
  _impl(0),
  _id(0),
  _hostAddressUInt32(0)
{
  _impl = new NetImpl();
  _preferredAddress = "*";
  initialize();
}

Net::~Net()
{
  while (_nicList.size() > 0)
  {
    Nic * nic = _nicList.front();
    _nicList.erase(_nicList.begin());
    delete nic;
  }

  delete _impl;

  // Just a dummy call
  // we want to make sure that all applications used this module have version data in their binaries
  extern std::string SaiGetVersion(); 
  SaiGetVersion();
}

#ifdef _WIN32
uint32_t inetPton(std::string ip)
{
  typedef union
  {
    uint8_t part[4];
    uint32_t i;
  }MyIp;

  char * ptr = (char*) ip.c_str();
  char * part[4] = {ptr, 0, 0, 0};
  char * dot = 0;
  dot = strchr(ptr, '.');
  part[1] = ptr = dot + 1; *dot = 0;
  dot = strchr(ptr, '.');
  part[2] = ptr = dot + 1; *dot = 0;
  dot = strchr(ptr, '.');
  part[3] = ptr = dot + 1; *dot = 0;

  MyIp myIp;
  myIp.part[0] = atoi(part[0]);
  myIp.part[1] = atoi(part[1]);
  myIp.part[2] = atoi(part[2]);
  myIp.part[3] = atoi(part[3]);
  return htonl(myIp.i);
}
#endif

void 
Net::initialize()
{
  const int MAX_INTERFACE_CARDS = 10;
  const int MAX_STRING_LEN = 1024;

  boost::basic_regex<char> *e = 0;
  if (_preferredAddress.compare("*") != 0)
  {
    e = new boost::basic_regex<char>(_preferredAddress.c_str());
  }

  Nic * selectedNic = 0;

  while (_nicList.size() > 0)
  {
    Nic * nic = _nicList.front();
    _nicList.erase(_nicList.begin());
    delete nic;
  }

#ifdef _WIN32
  int socketfd = 0;
  socketfd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
  if (socketfd < 0)
  {
    delete e;
    return;
  }

  INTERFACE_INFO interfaceInfo[MAX_INTERFACE_CARDS];
  unsigned long bytes;

  if ((WSAIoctl(socketfd, SIO_GET_INTERFACE_LIST, 0, 0, &interfaceInfo, sizeof(interfaceInfo), &bytes, 0, 0)) != 0)
  {
    closesocket(socketfd);
    delete e;
    return;
  }

  unsigned long num = bytes / sizeof(INTERFACE_INFO);

  char *interfaceAddr = new char[MAX_STRING_LEN];
  char *interfaceBcast= new char[MAX_STRING_LEN];
  char *interfaceName = new char[MAX_STRING_LEN];
  for (unsigned long i = 0; i < num; i += 1)
  {
    if (!(interfaceInfo[i].iiFlags & IFF_UP) || !(interfaceInfo[i].iiFlags & IFF_MULTICAST))
    {
      continue;
    }

    memset(interfaceAddr, 0, MAX_STRING_LEN);
    memset(interfaceBcast,0, MAX_STRING_LEN);
    memset(interfaceName, 0, MAX_STRING_LEN);

    sockaddr_in *addr, *bcast;
    addr  = (sockaddr_in *) &(interfaceInfo[i].iiAddress);
    bcast = (sockaddr_in *) &(interfaceInfo[i].iiBroadcastAddress);
    sprintf_s(interfaceAddr, MAX_STRING_LEN, "%s", inet_ntoa(addr->sin_addr));
    sprintf_s(interfaceBcast,MAX_STRING_LEN, "%s", inet_ntoa(bcast->sin_addr));

    Nic * nic = new Nic();
    nic->_name = interfaceName;
    nic->_ip   = interfaceAddr;
    nic->_bcast= interfaceBcast;
    _nicList.push_back(nic);

    if (sai::utils::Logger::GetInstance())
    {
      sai::utils::Logger::GetInstance()->print(
        sai::utils::Logger::SAILVL_INFO, "Detect a new iface card ('%s', '%s', '%s')\n",
        interfaceName, interfaceAddr, interfaceBcast);
    }

    if (selectedNic == 0 && e && boost::regex_match(interfaceAddr, *e))
    {
      selectedNic = nic;

      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_INFO, "'%s' is selected as a default iface card\n", interfaceAddr);
      }
    }
  }

  delete [] interfaceName;
  delete [] interfaceAddr;
  delete [] interfaceBcast;

  closesocket(socketfd);
#else
  int socketfd = 0;
  struct ifconf ifc;
  struct ifreq *ifr = 0;

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0)
  {
    delete e;
    return;
  }

  int len = MAX_INTERFACE_CARDS * sizeof(struct ifreq);
  int num = 0;

  ifr = (struct ifreq *) malloc (len);
  ifc.ifc_len = len;
  ifc.ifc_buf = (char*) ifr;

  if ((ioctl(socketfd, SIOCGIFCONF, &ifc)) < 0)
  {
    free(ifr);
    close(socketfd);
    delete e;
    return;
  }

  num = ifc.ifc_len / sizeof(struct ifreq);

  char *interfaceAddr = new char[MAX_STRING_LEN];
  char *interfaceBcast= new char[MAX_STRING_LEN];
  char *interfaceName = new char[MAX_STRING_LEN];

  for (int i = 0; i < num; i += 1)
  {
    struct ifreq req;
    memset(interfaceAddr, 0, MAX_STRING_LEN);
    memset(interfaceBcast,0, MAX_STRING_LEN);
    memset(interfaceName, 0, MAX_STRING_LEN);

    sprintf(interfaceName, "%s", ifc.ifc_req[i].ifr_name);
    memcpy(req.ifr_name, ifc.ifc_req[i].ifr_name, sizeof(req.ifr_name));

    if (ioctl(socketfd, SIOCGIFADDR, &req) < 0)
    {
      continue;
    }

    struct sockaddr_in * saddr = (struct sockaddr_in*)&(req.ifr_addr);
    if (inet_ntop(AF_INET, &(saddr->sin_addr), interfaceAddr, MAX_STRING_LEN) == NULL)
    {
      continue;
    }

    if (ioctl(socketfd, SIOCGIFBRDADDR, &req) < 0)
    {
      continue;
    }

    struct sockaddr_in * sbcast = (struct sockaddr_in*)&(req.ifr_broadaddr);
    if (inet_ntop(AF_INET, &(sbcast->sin_addr), interfaceBcast, MAX_STRING_LEN) == NULL)
    {
      continue;
    }

    Nic * nic = new Nic();
    nic->_name = interfaceName;
    nic->_ip   = interfaceAddr;
    nic->_bcast= interfaceBcast;
    _nicList.push_back(nic);

    if (sai::utils::Logger::GetInstance())
    {
      sai::utils::Logger::GetInstance()->print(
        sai::utils::Logger::SAILVL_INFO, "Detect a new iface card ('%s', '%s', '%s')\n",
        interfaceName, interfaceAddr, interfaceBcast);
    }

    if (selectedNic == 0 && e && boost::regex_match(interfaceAddr, *e))
    {
      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_INFO, "'%s' is selected as a default iface card\n", interfaceAddr);
      }

      selectedNic = nic;
    }
  }

  delete [] interfaceName;
  delete [] interfaceBcast;
  delete [] interfaceAddr;

  free(ifr);
  close(socketfd);
#endif

  if (selectedNic)
  {
    _hostAddress      = selectedNic->_ip;
    _hostBcastAddress = selectedNic->_bcast;
  }
  else
  {
    getHostAddress();
  }

  std::string ip = getLocalAddress();  
#ifdef _WIN32
  _hostAddressUInt32 = inetPton(ip);
#else
  struct in_addr addr;
  inet_pton(AF_INET, ip.c_str(), &addr);
  _hostAddressUInt32 = htonl(addr.s_addr);
#endif  
  time_t tim = time(0);
  sprintf(_sender, "%x%x", _hostAddressUInt32, tim);

  delete e;
}

void
Net::sortNicList(NicList& list)
{
  Nic ** nic = new Nic*[list.size()];

  NicListIterator iter;
  int i = 0;
  int size = list.size();
  if (list.size() > 0)
  {
    for (iter = list.begin(); iter != list.end(); iter++, i++)
    {
      nic[i] = *iter;
    }
  }

  list.clear();

  std::string priority [] = {"eth0", "eth1", "eth2", "eth3", "wlan0", "wlan1", "wlan2", "wlan3"};
  for (int j = 0; j < 8; j += 1)
  {
    for (i = 0; i < size; i += 1)
    {
      if (priority[j].length() > 0 && nic[i] && priority[j].compare(nic[i]->_name) == 0)
      {
        list.push_back(nic[i]);
        nic[i] = 0;
        priority[j].clear();
      }
    }
  }

  for (i = 0; i < size; i += 1)
  {
    if (nic[i])
    {
      list.push_back(nic[i]);
    }
  }

  delete [] nic;
}

void 
Net::getHostAddress()
{
  sortNicList(_nicList);

  std::string ip;
  std::string bcast;

  if (_nicList.size() > 0)
  {
    NicListIterator iter;
    for (iter = _nicList.begin(); iter != _nicList.end(); iter++)
    {
      Nic * nic = *iter;
      ip    = nic->_ip;
      bcast = nic->_bcast;
      if (ip.length() > 0 && ip.compare("127.0.0.1") != 0) 
      {
        break;
      }
      ip    = "";
      bcast = "";
    }
  }

  if (ip.length() == 0 && getNumNic() > 1)
  {
    std::string guess [] = {
      "eth0",  "eth1",  "eth2",  "eth3",  "eth4", 
      "wlan0", "wlan1", "wlan2", "wlan3", "wlan4", 
      "dmfe0", "hme0"};
    for (uint16_t i = 0; i < 12; i += 1)
    {
      ip = getLocalIpFromNic(guess[i]);
      // TODO : dealing with bcast
      if (ip.length() > 0) 
      {
        break;
      }
    }
  }

  if (ip.length() == 0 && getNumNic() >= 0)
  {
    ip = (_nicList.front())->_ip;
    // TODO : dealing with bcast
  }
  
  if (ip.length() == 0)
  {
    ip = getIpFromName("localhost");
    // TODO : dealing with bcast
  }

  if (ip.length() == 0)
  {
    ip = "127.0.0.1";
    // TODO : dealing with bcast
  }

  _hostAddress      = ip;
  _hostBcastAddress = bcast;
}

std::string 
Net::getIpFromName(std::string nm)
{
  static std::string ret; // Not a thread-safe var
  ret.clear();

  boost::asio::ip::tcp::resolver resolver(_impl->io);
  boost::asio::ip::tcp::resolver::query query(nm.c_str(), "http");
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
  boost::asio::ip::tcp::resolver::iterator end;


  boost::asio::ip::tcp::socket socket(_impl->io);
  boost::system::error_code error = boost::asio::error::host_not_found;
  while (error && iter != end)
  {
    socket.close();
    socket.connect(*iter++, error);
  }

  if (!error)
  {
    ret = socket.remote_endpoint().address().to_string();
  }

  return ret;
}

Net * 
Net::GetInstance()
{
  if (_instance == 0)
  {
    _instance = new Net();
  }
  return _instance;
}

uint32_t 
Net::getMessageId()
{
  // don't want to make it overflow and goes back to 0, so just reset 
  // it at some particular point
  if (++_id > 0xFFFFFFF0) 
  {
    _id = 1;
  }
  return _id;
}

std::string 
Net::getLocalIpFromNic(std::string name)
{
  static std::string ret;
  ret.clear();

  if (_nicList.size() > 0)
  {
    NicListIterator iter;
    for (iter = _nicList.begin(); iter != _nicList.end(); iter++)
    {
      Nic * nic = *iter;
      if (nic->_name.compare(name) == 0)
      {
        ret = nic->_ip;
        break;
      }
    }
  }
  return ret;
}

std::string 
Net::getNicList(std::string& ret)
{
  ret.clear();

  if (_nicList.size() > 0)
  {
    NicListIterator iter;
    for (iter = _nicList.begin(); iter != _nicList.end(); iter++)
    {
      Nic * nic = *iter;
      if (nic->_ip.compare(getLocalAddress()) == 0)
      {
        ret.append("selected,");
      }
      ret.append(nic->_ip);
      ret.append(",");
      ret.append(nic->_bcast);

      if ((iter + 1) != _nicList.end())
      {
        ret.append("!");
      }
    }
  }

  return ret;
}

void 
Net::mainLoop()
{
  _impl->io.run();
}

void 
Net::shutdown()
{
  _impl->io.stop();
}

void* 
Net::getIO()
{
  return (void*)_impl->getIO();
}

Nic::Nic()
{}

Nic::~Nic()
{}
