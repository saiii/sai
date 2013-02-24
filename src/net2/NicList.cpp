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

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <Objbase.h>
#endif
#include <string>
#include <boost/asio.hpp>
#include <boost/regex.hpp>

#include <net2/Nic.h>
#include "NicList.h"

using namespace sai::net2;

NicList::NicList():
  _defaultNic(0),
  _currentNic(0)
{
}

NicList::~NicList()
{
  while (_nicList.size() > 0)
  {
    Nic * nic = _nicList.front();
    _nicList.erase(_nicList.begin());
    delete nic;
  }
}

void 
NicList::detect()
{
  const int MAX_INTERFACE_CARDS = 10;
  const int MAX_STRING_LEN = 1024;

#ifdef _WIN32
  int socketfd = 0;
  socketfd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
  if(socket < 0)
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

  char *interfaceAddr  = (char*)::CoTaskMemAlloc(MAX_STRING_LEN);
  char *interfaceBcast = (char*)::CoTaskMemAlloc(MAX_STRING_LEN);

  for (unsigned long i = 0; i < num; i+=1)
  {
    if (!(interfaceInfo[i].iiFlags & IFF_UP) || !(interfaceInfo[i].iiFlags & IFF_MULTICAST))
    {
      continue;
    }

    memset(interfaceAddr, 0, MAX_STRING_LEN);
    memset(interfaceBcast,0, MAX_STRING_LEN);
    
    sockaddr_in *addr, *bcast;
    addr  = (sockaddr_in *) &(interfaceInfo[i].iiAddress);
    bcast = (sockaddr_in *) &(interfaceInfo[i].iiBroadcastAddress);
    sprintf_s(interfaceAddr, MAX_STRING_LEN, "%s", inet_ntoa(addr->sin_addr));
    sprintf_s(interfaceBcast,MAX_STRING_LEN, "%s", inet_ntoa(bcast->sin_addr));

    Nic * nic = new Nic();
    nic->_name.assign("Unknown", 8);
    nic->_ip.assign(interfaceAddr, strlen(interfaceAddr)+1);
    nic->_bcast.assign(interfaceBcast, strlen(interfaceBcast)+1);
    _nicList.push_back(nic); 
  }
  ::CoTaskMemFree(interfaceBcast);
  ::CoTaskMemFree(interfaceAddr);

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

  if((ioctl(socketfd, SIOCGIFCONF, &ifc)) < 0)
  {
    free(ifr);
    close(socketfd);
    return;
  }

  num = ifc.ifc_len / sizeof(struct ifreq);

  char *interfaceAddr  = new char[MAX_STRING_LEN];
  char *interfaceBcast = new char[MAX_STRING_LEN];
  char *interfaceName  = new char[MAX_STRING_LEN];

  for(int i = 0; i < num; i += 1)
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
    nic->_name.assign(interfaceName, strlen(interfaceName)+1);
    nic->_ip.assign(interfaceAddr, strlen(interfaceAddr)+1);
    nic->_bcast.assign(interfaceBcast, strlen(interfaceBcast)+1);
    _nicList.push_back(nic);
  }

  delete [] interfaceName;
  delete [] interfaceBcast;
  delete [] interfaceAddr;
  free(ifr);
  close(socketfd);
#endif
}

Nic * 
NicList::getDefaultNic()
{
  if (_defaultNic != 0)
  {
    return _defaultNic;
  }
  else
  {
    sortNicList(_nicList);
    if (_nicList.size() > 0)
    {
      _NicListIterator iter;
      for (iter = _nicList.begin(); iter != _nicList.end(); iter++)
      {
        Nic *nic = *iter;
        std::string ip = nic->_ip;
        if (ip.length() > 0 && strcmp(ip.c_str(), "127.0.0.1") != 0)
        {
          _currentNic = nic;
          _defaultNic = nic;
          return _defaultNic; 
        }
      }
    }
    return 0;
  }
}

Nic * 
NicList::getNic(const char * ip)
{ 
  if (_nicList.size() <= 0)
  {
    return 0;
  } 
  
  _NicListIterator iter;
  
  for (iter = _nicList.begin(); iter != _nicList.end(); iter += 1)
  {
    Nic * nic = *iter;
    if (nic->_ip.compare(ip) == 0)
    {
      _currentNic = nic;
      return nic;
    }
  }
  return 0;
}

void 
NicList::sortNicList(_NicList& list)
{
  Nic ** nic = new Nic*[list.size()];

  _NicListIterator iter;
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
    for (int i = 0; i < size; i += 1)
    {
      if(priority[j].length() > 0 && nic[i] && strcmp(priority[j].c_str(), nic[i]->_name.c_str()) == 0)
      {
        list.push_back(nic[i]);
        nic[i] = 0;
        priority[j].clear();
      }
    }
  }
  
  for (int i = 0; i < size; i += 1)
  {
    if (nic[i])
    {
      list.push_back(nic[i]);
    }
  }
  
  delete [] nic;
}
