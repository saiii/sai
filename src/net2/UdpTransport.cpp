//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Objbase.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#endif

#include <iostream>

#include <utils/ThreadPool.h>
#include <utils/Logger.h>

#include <net2/Net.h>
#include <net2/Nic.h>
#include <net2/NicList.h>
#include "Transport.h"

#define NET_BUFFER_SIZE1  40960
#define NET_BUFFER_SIZE2 819200

using namespace sai::net2;

namespace sai { 
namespace net2 {

class UdpTransportImpl
{
public:
  bool                useLAN;
  struct sockaddr_in  lAddr;
  struct sockaddr_in  destinationAddr;

public:
  UdpTransportImpl():
    useLAN(false)
  {
    if (!UdpSocket::_instance)
    {
      UdpSocket::_instance = new UdpSocket();
    }
  }

  ~UdpTransportImpl()
  {
  }

  void bind(std::string ip)
  {
#if 0
    memset(&lAddr, 0, sizeof(lAddr));
    lAddr.sin_family      = AF_INET;
    lAddr.sin_addr.s_addr = ip.compare("0.0.0.0") != 0 ? inet_addr(ip.c_str()) : htonl(INADDR_ANY);
    lAddr.sin_port        = htons(0);

    if (useLAN)
      ::bind(UdpSocket::_instance->_lSendFd, (struct sockaddr *) &lAddr, sizeof(lAddr));
    else
      ::bind(UdpSocket::_instance->_iSendFd, (struct sockaddr *) &lAddr, sizeof(lAddr));

    if (sai::utils::Logger::GetInstance())
    {
      sai::utils::Logger::GetInstance()->print(
        sai::utils::Logger::SAILVL_DEBUG, "UdpTransportImpl bind %s\n", ip.c_str());
    }
#endif
  }

  void destination(std::string ip, uint16_t port)
  {
    char * ptr = (char*) ip.c_str();
    char * dot = strchr(ptr, '.');
    char first [4];
    memcpy(first, ptr, dot - ptr);
    first[3] = 0;
    uint16_t num = atoi(first);
    if (num >= 224 && num <= 239)
    {
      useLAN = true;
    }

    memset(&destinationAddr, 0, sizeof(destinationAddr));
    destinationAddr.sin_family      = AF_INET;
    destinationAddr.sin_addr.s_addr = ip.compare("0.0.0.0") != 0 ? inet_addr(ip.c_str()) : htonl(INADDR_ANY);
    destinationAddr.sin_port        = htons(port);
  }

  void send(const char *data, uint32_t size)
  {
    size_t bytes = 0;

#if DEBUG_NET
    static uint16_t seqNo = 54300000;
    seqNo++;
    printf("Seq %u, Size %u\n", seqNo, size);
    uint16_t nSeqNo = htons(seqNo);
    memcpy((char*)(data + 10), &nSeqNo, sizeof(nSeqNo));
#endif

    if (useLAN)
      bytes = sendto(UdpSocket::_instance->_lSendFd, data, size, 0, (struct sockaddr *)&destinationAddr, sizeof(destinationAddr));
    else 
      bytes = sendto(UdpSocket::_instance->_iSendFd, data, size, 0, (struct sockaddr *)&destinationAddr, sizeof(destinationAddr));

    if (bytes != size)
    {
      //fprintf(stderr, "Sending error!\n");
    }
  }

  void send(const std::string data)
  {
    if (useLAN)
      sendto(UdpSocket::_instance->_lSendFd, data.data(), data.size(), 0, (struct sockaddr *)&destinationAddr, sizeof(destinationAddr));
    else 
      sendto(UdpSocket::_instance->_iSendFd, data.data(), data.size(), 0, (struct sockaddr *)&destinationAddr, sizeof(destinationAddr));
  }
};

class BufferManager
{
private:
  char*                         buffer;
  char*                         buffer2;
  uint32_t                      &buffer2Size;

public:
  RawDataHandler*               handler;

public:
  BufferManager(char * buf, char * buf2, uint32_t& size, RawDataHandler * hdlr);
  ~BufferManager();
  void processBuffer(Endpoint& endpoint, char * ptr, int32_t bytes_received);
};

class UdpTransportThread : public sai::utils::ThreadTask
{
public:
  int             sock;
  bool            enable; 
  bool            isBlocking;
  char          * udpBuffer;
  char          * udpBuffer2;
  uint32_t        udpBuffer2Size;
  RawDataHandler* handler; 
  BufferManager*  manager;

public:
  UdpTransportThread(int sck, RawDataHandler* h, bool blocking) : 
    sock(sck), 
    enable(true), 
    isBlocking(blocking),
    udpBuffer2Size(0),
    handler(h),
    manager(0)
  {
#ifdef _WIN32
    udpBuffer = (char*)::CoTaskMemAlloc(NET_BUFFER_SIZE2);
    udpBuffer2= (char*)::CoTaskMemAlloc(NET_BUFFER_SIZE2);
#else
    udpBuffer = new char[NET_BUFFER_SIZE2];
    udpBuffer2= new char[NET_BUFFER_SIZE2];
#endif
    manager = new BufferManager(udpBuffer, udpBuffer2, udpBuffer2Size, handler);
  }
  ~UdpTransportThread() 
  {
#ifdef _WIN32
    ::CoTaskMemFree(udpBuffer2);
    ::CoTaskMemFree(udpBuffer);
#else
    delete [] udpBuffer2;
    delete [] udpBuffer;
#endif
    delete manager;
  }
  void threadEvent()
  {
    struct sockaddr_in fromAddr;
#ifdef _WIN32
    int32_t fromLen = sizeof(fromAddr);
#else
    socklen_t fromLen = sizeof(fromAddr);
#endif

    if (!isBlocking)
    {
      struct timeval tv;
      fd_set readfds;
  
      {
        memset(&fromAddr, 0, fromLen);
        int bytes_recvd = -1;
        if ((bytes_recvd = recvfrom(sock, udpBuffer, NET_BUFFER_SIZE1, 0, (struct sockaddr*)&fromAddr, &fromLen)) > 0) 
        {
          Endpoint endpoint(inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port), UDP, 0);
          manager->processBuffer(endpoint, udpBuffer, bytes_recvd);
        }
      }
  
      while (enable)
      {
        int max = sock;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
  
        tv.tv_sec  = 0;
        tv.tv_usec = 0;
        int status = select(max + 1, &readfds, NULL, NULL, &tv);
        if (status > 0 && FD_ISSET(sock, &readfds))
        {
          memset(&fromAddr, 0, fromLen);
          int bytes_recvd = -1;
          if ((bytes_recvd = recvfrom(sock, udpBuffer, NET_BUFFER_SIZE1, 0, (struct sockaddr*)&fromAddr, &fromLen)) > 0) 
          {
            Endpoint endpoint(inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port), UDP, 0);
            manager->processBuffer(endpoint, udpBuffer, bytes_recvd);
          }
        }
      }
    }
    else
    {
      while (enable)
      {
        memset(&fromAddr, 0, fromLen);
        int bytes_recvd = -1;
        if ((bytes_recvd = recvfrom(sock, udpBuffer, NET_BUFFER_SIZE1, 0, (struct sockaddr*)&fromAddr, &fromLen)) > 0) 
        {
          Endpoint endpoint(inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port), UDP, 0);
          manager->processBuffer(endpoint, udpBuffer, bytes_recvd);
        }
      }
    }
  }
};

class UdpReceiver
{
friend class InternalTransportImpl;
private:
  int         _sock;
  uint16_t    _port;
  std::string _ip;

public:
  UdpReceiver() : _sock(-1), _port(0)
  {  
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock < 0)
    {
      exit(1);
    }
  }
  ~UdpReceiver()
  {
#ifdef _WIN32
    closesocket(_sock);
#else
    close(_sock);
#endif  
  }
  void initUDPSocket(bool isBlocking);
};


}}

UdpSocket * UdpSocket::_instance = 0;

UdpSocket::UdpSocket()
{
  _iSendFd = _lSendFd = -1;
  _iSendFd = socket(AF_INET, SOCK_DGRAM, 0);
  _lSendFd = socket(AF_INET, SOCK_DGRAM, 0);

  if (_iSendFd < 0 || _lSendFd < 0)
  {
    exit(1);
  }

  init();

#if 0
  uint8_t ttl = 1;
  setsockopt(_iSendFd, IPPROTO_IP, IP_MULTICAST_TTL, (char*) &ttl, sizeof(ttl));
  setsockopt(_lSendFd, IPPROTO_IP, IP_MULTICAST_TTL, (char*) &ttl, sizeof(ttl));

  uint8_t loop = 0;
  setsockopt(_iSendFd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop));
  setsockopt(_lSendFd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop));
#endif
}

UdpSocket::~UdpSocket()
{
  UdpSocket::_instance = 0;

#ifdef _WIN32
  closesocket(_iSendFd);
  closesocket(_lSendFd);
#else
  close(_iSendFd);
  close(_lSendFd);
#endif  
  _iSendFd = _lSendFd = -1;
}

void 
UdpSocket::init()
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  addr.sin_port        = htons(0); 

  if ((bind(_iSendFd, (struct sockaddr *) &addr, sizeof(addr))) < 0) 
  {
    // TODO: Log error message
    return;
  }

  if ((bind(_lSendFd, (struct sockaddr *) &addr, sizeof(addr))) < 0) 
  {
    // TODO: Log error message
    return;
  }

  //int recvBufferSize = NET_BUFFER_SIZE1;

  //if (setsockopt(_iSendFd, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, sizeof(recvBufferSize)) == -1) 
  //{
  //  // TODO: Log error message
  //  return;
  //}

  //if (setsockopt(_iSendFd, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(sendBufferSize)) == -1) 
  //{
  //  // TODO: Log error message
  //  return;
  //}

  //if (setsockopt(_lSendFd, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, sizeof(recvBufferSize)) == -1) 
  //{
  //  // TODO: Log error message
  //  return;
  //}

  //if (setsockopt(_lSendFd, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(sendBufferSize)) == -1) 
  //{
  //  // TODO: Log error message
  //  return;
  //}

  uint8_t loop = 0;
  setsockopt(_iSendFd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop));
  setsockopt(_lSendFd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop));

#ifdef _WIN32
  DWORD mtuflag = 1;
#else
  int mtuflag = IP_PMTUDISC_DONT;
#endif
  if (setsockopt(_iSendFd, 
                 IPPROTO_IP,
#ifdef _WIN32
		 IP_DONTFRAGMENT,
#else
                 IP_MTU_DISCOVER, 
#endif
                 (char *)&mtuflag, 
                 sizeof(mtuflag)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  uint8_t ttl = 1;
  if (setsockopt(_iSendFd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) == -1) 
  {
    // TODO: Log error message
    return;
  }
 
  if (setsockopt(_lSendFd, 
                 IPPROTO_IP,
#ifdef _WIN32
		 IP_DONTFRAGMENT,
#else
                 IP_MTU_DISCOVER, 
#endif
                 (char *)&mtuflag, 
                 sizeof(mtuflag)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  if (setsockopt(_lSendFd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  int dscp = 0;
  if (setsockopt(_iSendFd, IPPROTO_IP, IP_TOS, (char *)&dscp, sizeof(dscp)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  if (setsockopt(_lSendFd, IPPROTO_IP, IP_TOS, (char *)&dscp, sizeof(dscp)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  
#ifdef _WIN32
  u_long flag = 1;
  if (ioctlsocket(_lSendFd, FIONBIO, &flag) != 0) 
  {
    // TODO: Log error message
    return;
  }
  if (ioctlsocket(_iSendFd, FIONBIO, &flag) != 0) 
  {
    // TODO: Log error message
    return;
  }
#else
  int flag = 1;
  if ((flag = fcntl(_lSendFd, F_GETFL)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  flag |= O_NONBLOCK;
  if (fcntl(_lSendFd, F_SETFL, flag) == -1) 
  {
    // TODO: Log error message
    return;
  }

  flag = 1;
  if ((flag = fcntl(_iSendFd, F_GETFL)) == -1) 
  {
    // TODO: Log error message
    return;
  }

  flag |= O_NONBLOCK;
  if (fcntl(_iSendFd, F_SETFL, flag) == -1) 
  {
    // TODO: Log error message
    return;
  }
#endif
}

UdpTransport::UdpTransport():
  _impl(0)
{
  _impl = new UdpTransportImpl();
}

UdpTransport::~UdpTransport()
{
  delete _impl;
}

void 
UdpTransport::bind(std::string ip)
{
  _impl->bind(ip);
}

void 
UdpTransport::close()
{
}

void 
UdpTransport::destination(std::string ip, uint16_t port)
{
  _impl->destination(ip, port);
}

void
UdpTransport::send(const char * data, uint32_t size)
{
  _impl->send(data, size);
}

void
UdpTransport::send(std::string data)
{
  _impl->send(data);
}

