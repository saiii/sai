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

#ifndef __SAI_NET_SOCKET__
#define __SAI_NET_SOCKET__
#include <stdint.h>
#include <string>
#include <net/Net.h>

namespace sai 
{ 
namespace net 
{

typedef enum
{
  SAI_SOCKET_EOA,
  SAI_SOCKET_B_REUSE_ADDR,
  SAI_SOCKET_I_PROTOCOL,
  SAI_SOCKET_B_USEIP_MULTICAST
}SocketOptions;

typedef enum
{
  SAI_SOCKET_OPT_FALSE = 0,
  SAI_SOCKET_OPT_TRUE  = 1
}SocketOptionBoolean;

typedef enum
{
  SAI_SOCKET_OPT_UDP = 0,
  SAI_SOCKET_OPT_TCP = 1
}SocketOptionProtocol;

class SocketEventHandler;
class Socket
{
protected:
  Net& _net;

protected:
  Socket(Net& net);

public:
  virtual ~Socket();

  virtual void open() {}
  virtual void close() {}
  virtual void setEventHandler(SocketEventHandler *) {}
};

class ClientSocket;
class SocketEventHandler
{
public:
  virtual bool processConnectionEvent(std::string ip) { return true; }
  virtual void processConnectedEvent(ClientSocket * socket) {}
  virtual void processDataEvent(char *, uint32_t) {}
  virtual void processConnectionClosedEvent() {}
};

class Net;
class ServerSocket : public Socket
{
protected:
  ServerSocket(Net& net);
  
public:
  virtual ~ServerSocket();
  static ServerSocket * Create(Net&, SocketOptions, ...); 

  virtual void listen()  {}
  virtual void join(std::string mcast) {}
  virtual void bind(std::string ip, uint16_t port) {}
  virtual void setEventHandler(SocketEventHandler *) {}
};

class ClientSocket : public Socket
{
protected:
  ClientSocket(Net& net);

public:
  virtual ~ClientSocket();
  static ClientSocket * Create(Net&, SocketOptions, ...);

  virtual void connect(std::string ip, uint16_t port) {}
  virtual void send(const char *data, uint32_t size) {}
  virtual void send(const std::string data) {}
  virtual void bind(std::string ip) {}
};

}
}
#endif
