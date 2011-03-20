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

#include "Net.h"

using namespace sai::net;

Net * Net::_instance = 0;

Net::Net(boost::asio::io_service& io):
  _io(io),
  _id(0)
{
  initialize();
  getHostAddress();

  std::string ip = getLocalAddress();
  struct in_addr addr;
  inet_pton(AF_INET, ip.c_str(), &addr);
  uint32_t ipaddr = htonl(addr.s_addr);
  uint32_t tim    = time(0);
  sprintf(_sender, "%x%x", ipaddr, tim);

  _instance = this;
}

Net::~Net()
{
  while (_nicList.size() > 0)
  {
    Nic * nic = _nicList.front();
    _nicList.erase(_nicList.begin());
    delete nic;
  }

  // Just a dummy call
  // we want to make sure that all applications used this module have version data in their binaries
  extern std::string GetVersion(); 
  GetVersion();
}

void 
Net::initialize()
{
  const int MAX_INTERFACE_CARDS = 10;
  const int MAX_STRING_LEN = 1024;

#ifdef _WIN32
  int socketfd = 0;
  socketfd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
  if (socketfd < 0)
  {
    return;
  }

  INTERFACE_INFO interfaceInfo[MAX_INTERFACE_CARDS];
  unsigned long bytes;

  if ((WSAIoctl(socketfd, SIO_GET_INTERFACE_LIST, 0, 0, &interfaceInfo, sizeof(interfaceInfo), &bytes, 0, 0)) != 0)
  {
    closesocket(socketfd);
    return;
  }

  unsigned long num = bytes / sizeof(INTERFACE_INFO);

  char *interfaceAddr = new char[MAX_STRING_LEN];
  char *interfaceName = new char[MAX_STRING_LEN];
  for (unsigned long i = 0; i < num; i += 1)
  {
    memset(interfaceAddr, 0, MAX_STRING_LEN);
    memset(interfaceName, 0, MAX_STRING_LEN);

    sockaddr_in *addr;
    addr = (sockaddr_in *) &(interfaceInfo[i].iiAddress);
    sprintf_s(interfaceAddr, MAX_STRING_LEN, "%s", inet_ntoa(addr->sin_addr));

    Nic * nic = new Nic();
    nic->_name = interfaceName;
    nic->_ip   = interfaceAddr;
    _nicList.push_back(nic);
  }

  delete [] interfaceName;
  delete [] interfaceAddr;

  closesocket(socketfd);
#else
  int socketfd = 0;
  struct ifconf ifc;
  struct ifreq *ifr = 0;

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0)
  {
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
    return;
  }

  num = ifc.ifc_len / sizeof(struct ifreq);

  char *interfaceAddr = new char[MAX_STRING_LEN];
  char *interfaceName = new char[MAX_STRING_LEN];

  for (int i = 0; i < num; i += 1)
  {
    struct ifreq req;
    memset(interfaceAddr, 0, MAX_STRING_LEN);
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

    Nic * nic = new Nic();
    nic->_name = interfaceName;
    nic->_ip   = interfaceAddr;
    _nicList.push_back(nic);
  }

  delete [] interfaceName;
  delete [] interfaceAddr;

  free(ifr);
  close(socketfd);
#endif
}

void 
Net::getHostAddress()
{
  std::string ip;

  if (getNumNic() > 1)
  {
    std::string guess [] = {"eth0", "wlan0", "dmfe0", "hme0"};
    for (uint16_t i = 0; i < 4; i += 1)
    {
      ip = getLocalIpFromNic(guess[i]);
      if (ip.length() > 0) 
      {
        break;
      }
    }
  }

  if (ip.length() == 0 && getNumNic() >= 0)
  {
    ip = get1stLocalIp();
  }
  
  if (ip.length() == 0)
  {
    ip = getIpFromName("localhost");
  }

  if (ip.length() == 0)
  {
    ip = "127.0.0.1";
  }

  _hostAddress = ip;
}

std::string 
Net::getIpFromName(std::string nm)
{
  static std::string ret; // Not a thread-safe var
  ret.clear();

  boost::asio::ip::tcp::resolver resolver(_io);
  boost::asio::ip::tcp::resolver::query query(nm.c_str(), "http");
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
  boost::asio::ip::tcp::resolver::iterator end;


  boost::asio::ip::tcp::socket socket(_io);
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
  return ret;
}

std::string 
Net::get1stLocalIp()
{
  Nic * nic = _nicList.front();
  return nic->_ip;
}

Nic::Nic()
{}

Nic::~Nic()
{}