// SAI [ 14 Oct 2009 ]
#ifndef __SAI_SOCKET__
#define __SAI_SOCKET__
#include <stdint.h>
#include <string>

namespace sai { namespace net {

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
  Socket();

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
  ServerSocket();
  
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
  ClientSocket();

public:
  virtual ~ClientSocket();
  static ClientSocket * Create(Net&, SocketOptions, ...);

  virtual void connect(std::string ip, uint16_t port) {}
  virtual void send(const char *data, uint32_t size) {}
  virtual void send(const std::string data) {}
  virtual void bind(std::string ip) {}
};

}}
#endif
