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

#ifndef _WIN32
#include <syslog.h>
#endif

#include <stdarg.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Exception.h"
#include "Socket.h"
#include "Net.h"

#define TCP_BUFFER_SIZE 8192

using namespace sai::net;

class UDPMcastServerSocket : public ServerSocket
{
private:
  boost::asio::ip::udp::socket   _socket;
  boost::asio::ip::udp::endpoint _endpoint;
  boost::asio::ip::udp::endpoint _senderEndpoint;
  char              *_buffer;
  uint32_t           _bufferSize;
  SocketEventHandler *_handler;

public:
  bool        reuseAddr;
  bool        openned;

public:
  UDPMcastServerSocket(Net& net) : 
    ServerSocket(net),
    _socket(*((boost::asio::io_service*)net.getIO())),
    _buffer(0),
    _bufferSize(65536),
    _handler(0),
    reuseAddr(false), 
    openned(false)
  {
    _buffer = new char[_bufferSize];
  }
  ~UDPMcastServerSocket() 
  {
    delete [] _buffer;
  }

  void open() 
  {
    _socket.open(_endpoint.protocol());
    _socket.set_option(boost::asio::ip::udp::socket::reuse_address(reuseAddr));
#if 0
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
	addr.sin_port = htons(_endpoint.port());
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//addr.sin_addr.s_addr = _endpoint.address().to_string().compare("0.0.0.0") == 0 ? htonl(INADDR_ANY) : inet_addr(_endpoint.address().to_string().c_str());

	if (::bind(_socket.native(),(struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
	   _socket.close();
      return;
    }
#else
    _socket.bind(_endpoint);
#endif
    openned = true;
  }

  void close() 
  {
    openned = false;
    try 
    {
#ifndef _WIN32
      _socket.cancel();
#endif
      _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
      _socket.close();
    } catch(boost::system::system_error& e)
    {
#ifdef _WIN32
#else
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "Failed to close a UDPMcastServerSocket");
    syslog(LOG_ERR, "%s", e.what());
    closelog();
#endif
    }
  }

  void listen()  
  { 
    if (_handler == 0)
    {
      throw SocketException("SocketEventHandler cannot be null!");
    }

    _socket.async_receive_from(
      boost::asio::buffer(_buffer, _bufferSize), _senderEndpoint,
      boost::bind(&UDPMcastServerSocket::processDataEvent, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }

  void join(std::string mcast) 
  {
    if (!openned) throw SocketException("Invalid state! The socket must be openned before!");

#if 1
   struct ip_mreq imreq;
   memset(&imreq, 0, sizeof(struct ip_mreq));

   imreq.imr_multiaddr.s_addr = inet_addr(mcast.c_str());
   imreq.imr_interface.s_addr = inet_addr(_endpoint.address().to_string().c_str()); 
   //imreq.imr_interface.s_addr = INADDR_ANY; 

   int ret = setsockopt(_socket.native(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&imreq, sizeof(struct ip_mreq));
   if (ret != 0)
   {
	 std::string err = "Unable to join the specified multicast group";
     throw SocketException(err);
   }
#else

    std::string err = "";
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(mcast);
    try
    {
      _socket.set_option(boost::asio::ip::multicast::join_group(address));
    } catch(boost::system::system_error& e)
    {
      err = e.what();
    }
    if (err.length() > 0)
      throw SocketException(err); 
#endif
  }

  void bind(std::string ip, uint16_t p) 
       { 
         const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
         _endpoint.address(address);
         _endpoint.port(p);
       }

  void setEventHandler(SocketEventHandler *handler) 
       {
         _handler = handler;
       }

  void processDataEvent(const boost::system::error_code& error, size_t bytes_recvd)
       {
         if (error) 
         {
           return;
         }

         _handler->processDataEvent(_buffer, bytes_recvd);
         listen();
       }
};

ServerSocket::ServerSocket(Net& net):
  Socket(net)
{}

ServerSocket::~ServerSocket()
{}

class TCPClientSocket : public ClientSocket
{
protected:
  class _State
  {
    protected:
      TCPClientSocket * _client;

    public:
      _State(TCPClientSocket * c):_client(c) {}
      virtual ~_State() {}
      virtual void connect(std::string ip, uint16_t port) {}
      virtual void send(const char * data, uint32_t size) {}
      virtual void send(const std::string data) {}
      virtual void open() {}
      virtual void close() {}
      virtual void setEventHandler(SocketEventHandler *) {} 
  };

  class _ReadyState;
  class _NilState : public _State
  {
    public:
      _NilState(TCPClientSocket * c):_State(c) {}
      ~_NilState() {}
      void connectedEvent(const boost::system::error_code& err)
      {
        if (!err)
        {
          _ReadyState * state = dynamic_cast<_ReadyState*>(_client->_ready);
          _client->_state = state; 
          state->readPreparation();
          if (_client->_handler)
          {
            _client->_handler->processConnectedEvent(_client);
          }
        }
      }
      void connect(std::string ip, uint16_t port) 
      {
        const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
        boost::asio::ip::tcp::endpoint endpoint(address, port);

        _client->_socket.async_connect(endpoint,
          boost::bind(&_NilState::connectedEvent, this, boost::asio::placeholders::error));
      }
      void open() {}
      void setEventHandler(SocketEventHandler *h) 
      { _client->_handler = h; } 
  };
  
  class _ReadyState : public _State
  {
    public:
      char     *buffer;
      uint32_t  bufferSize;

    public:
      _ReadyState(TCPClientSocket * c):
        _State(c),
        buffer(0),
        bufferSize(TCP_BUFFER_SIZE) 
      {
        buffer = new char[bufferSize];
      }
      ~_ReadyState() 
      {
        delete [] buffer;
      }
      void sentEvent(const boost::system::error_code& err) 
      {
        if (err)
        {
          close();
        }
      }
      void send(const char * data, uint32_t size) 
      {
        boost::asio::async_write(_client->_socket, boost::asio::buffer(data, size),
          boost::bind(&_ReadyState::sentEvent, this, boost::asio::placeholders::error));
      }
      void send(const std::string data) 
      {
        boost::asio::async_write(_client->_socket, 
          boost::asio::buffer(data.data(), data.size()),
          boost::bind(&_ReadyState::sentEvent, this, boost::asio::placeholders::error));
      }
      void readPreparation()
      {
        _client->_socket.async_read_some(
          boost::asio::buffer(buffer, bufferSize), 
          boost::bind(&TCPClientSocket::_ReadyState::processDataEvent, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
      }
      void processDataEvent(const boost::system::error_code& err, size_t bytes)
      {
        if (!err)
        {
          if (_client->_handler) _client->_handler->processDataEvent(buffer, bytes);
          readPreparation();
        }
        else
        {
          close();
        }
      }
      void close() 
      {
        try
        {
#ifndef _WIN32
          _client->_socket.cancel();
#endif
          _client->_socket.shutdown(boost::asio::socket_base::shutdown_both);
          _client->_socket.close();
          _client->_state = _client->_nil;

          if (_client->_handler) _client->_handler->processConnectionClosedEvent();
        }catch(boost::system::system_error& e)
        {
#ifdef _WIN32
#else
          openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
          syslog(LOG_ERR, "Failed to close a TCPClientSocket");
          syslog(LOG_ERR, "%s", e.what());
          closelog();
#endif
        }
      }
  };
protected:
  friend class _NilState;
  friend class _ReadyState;
  friend class TCPServerSocket;

  boost::asio::ip::tcp::socket   _socket;
  boost::asio::ip::tcp::resolver _resolver;
  _State             * _nil;
  _State             * _ready;
  _State             * _state;
  SocketEventHandler * _handler;

public:
  TCPClientSocket(Net& net):
    ClientSocket(net),
    _socket(*((boost::asio::io_service*)net.getIO())),
    _resolver(*((boost::asio::io_service*)net.getIO())),
    _handler(0)
  {
    _nil   = new _NilState(this);
    _ready = new _ReadyState(this);
    _state = _nil;
  }
  ~TCPClientSocket()
  {
    delete _nil;
    delete _ready;
  }
  void connect(std::string ip, uint16_t port) { _state->connect(ip, port); }
  void send(const char *data, uint32_t size)  { _state->send(data, size);  }
  void send(const std::string data)           { _state->send(data);        }
  void open()   { _state->open();  }
  void close()  { _state->close(); }
  void setEventHandler(SocketEventHandler *h)   { _state->setEventHandler(h); } 
};

class TCPServerSocket : public ServerSocket
{
private:
  class _Session : public TCPClientSocket
  {
    public:
      char     *buffer;
      uint32_t  bufferSize;

    public:
      _Session(Net& net) : TCPClientSocket(net), bufferSize(TCP_BUFFER_SIZE)
      {
        buffer = new char[bufferSize];
      }
      ~_Session()
      {
        delete [] buffer;
      }
      void start()
      {
        _socket.async_read_some(boost::asio::buffer(buffer, bufferSize), 
          boost::bind(&TCPServerSocket::_Session::processDataEvent, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
      }
      void processDataEvent(const boost::system::error_code& err, size_t bytes_transferred)
      {
        if (!err)
        {
          if (_handler) _handler->processDataEvent(buffer, bytes_transferred);
          start();
        }
        else
        {
          delete this;
        }
      }
  };

private:
  Net&                            _net;
  boost::asio::ip::tcp::acceptor *_acceptor;
  boost::asio::ip::tcp::endpoint  _endpoint;
  SocketEventHandler             *_handler;

public:
  bool        reuseAddr;

public:
  TCPServerSocket(Net& net):
    ServerSocket(net),
    _net(net), 
    _acceptor(0),
    _handler(0),
    reuseAddr(false)
  {
  }
  ~TCPServerSocket()
  {
    if (_acceptor) 
    {
      close();
    }
  }

  void processConnectionEvent(_Session * session, const boost::system::error_code& err)
  {
    if (!err)
    {
      bool acceptOk = true;
      if (_handler)
      {
        boost::asio::ip::tcp::endpoint endpoint = session->_socket.remote_endpoint();
        if (!_handler->processConnectionEvent(endpoint.address().to_string()))
        {
          acceptOk = false;
        }
      }

      if (acceptOk)
      {
        if (_handler)
        {
          _handler->processConnectedEvent(session);
        }
        session->_state = session->_ready;
        session->start();
      }
      else
      {
        delete session;
      }

      _Session * sess = new _Session(_net);
      _acceptor->async_accept(sess->_socket,
        boost::bind(&TCPServerSocket::processConnectionEvent, 
                    this, sess, boost::asio::placeholders::error));
    }
    else
    {
      delete session;
    } 
  }

  void listen()
  {
    _Session * session = new _Session(_net);
    _acceptor->async_accept(session->_socket,
      boost::bind(&TCPServerSocket::processConnectionEvent, 
                  this, session, boost::asio::placeholders::error));
  }

  void bind(std::string ip, uint16_t port)
  {
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    _endpoint.address(address);
    _endpoint.port(port);
  }

  void setEventHandler(SocketEventHandler * h)
  {
    _handler = h;
  }

  void open()
  {
    _acceptor = new boost::asio::ip::tcp::acceptor(
      *((boost::asio::io_service*)_net.getIO()), _endpoint);
    _acceptor->set_option(boost::asio::ip::tcp::socket::reuse_address(reuseAddr));
  }

  void close()
  {
    try 
    {
      _acceptor->cancel();
      _acceptor->close();
      delete _acceptor;
      _acceptor = 0;
    } catch(boost::system::system_error& e)
    {
#ifdef _WIN32
#else
      openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
      syslog(LOG_ERR, "Failed to close a TCPServerSocket");
      syslog(LOG_ERR, "%s", e.what());
      closelog();
#endif
    }
  }
};


class UDPMcastClientSocket : public ClientSocket
{
private:
  boost::asio::ip::udp::endpoint  _remoteEndPoint;
  boost::asio::ip::udp::socket    _socket;
  bool                            _openned;

public:
  UDPMcastClientSocket(Net& net):
    ClientSocket(net),
    _socket(*((boost::asio::io_service*)net.getIO()), _remoteEndPoint.protocol()),
    _openned(true)
  {
    boost::asio::ip::udp::endpoint endp;
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(Net::GetInstance()->getLocalAddress());
    endp.address(address);
    _socket.bind(endp);
  }

  ~UDPMcastClientSocket()
  {}

  void open() 
  {
    _openned = true;
  }

  void close()
  {
    _openned = false;
    try
    {
#ifndef _WIN32
      _socket.cancel();
#endif
      _socket.shutdown(boost::asio::socket_base::shutdown_both);
      _socket.close();
    }catch(boost::system::system_error& e)
    {
#ifdef _WIN32
#else
      openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
      syslog(LOG_ERR, "Failed to close a UDPMcastClientSocket");
      syslog(LOG_ERR, "%s", e.what());
      closelog();
#endif
    }
  }

  void connect(std::string ip, uint16_t port) 
  {
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    _remoteEndPoint.address(address);
    _remoteEndPoint.port(port);
  }

  void send(const char *data, uint32_t size) 
  {
    _socket.async_send_to(boost::asio::buffer(data, size), _remoteEndPoint,
      boost::bind(&UDPMcastClientSocket::processDataSentEvent, this,
      boost::asio::placeholders::error));
  }

  void send(const std::string data) 
  {
    _socket.async_send_to(boost::asio::buffer(data.c_str(), data.size()), _remoteEndPoint,
      boost::bind(&UDPMcastClientSocket::processDataSentEvent, this,
      boost::asio::placeholders::error));
  }

  void processDataSentEvent(const boost::system::error_code& error)
  {
  }
};

class UDPBcastClientSocket : public ClientSocket
{
private:
  boost::asio::ip::udp::endpoint  _remoteEndPoint;
  boost::asio::ip::udp::socket    _socket;
  bool                            _openned;

public:
  UDPBcastClientSocket(Net& net):
    ClientSocket(net),
    _socket(*((boost::asio::io_service*)net.getIO()), _remoteEndPoint.protocol()),
    _openned(true)
  {
    boost::asio::ip::udp::endpoint endp;
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(Net::GetInstance()->getLocalAddress());
    endp.address(address);

    boost::asio::socket_base::broadcast option(true);
    _socket.set_option(option);
    _socket.bind(endp);
  }

  ~UDPBcastClientSocket()
  {}

  void open() 
  {
    _openned = true;
  }

  void close()
  {
    _openned = false;
    try
    {
#ifndef _WIN32
      _socket.cancel();
#endif
      _socket.shutdown(boost::asio::socket_base::shutdown_both);
      _socket.close();
    }catch(boost::system::system_error& e)
    {
#ifdef _WIN32
#else
      openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
      syslog(LOG_ERR, "Failed to close a UDPBcastClientSocket");
      syslog(LOG_ERR, "%s", e.what());
      closelog();
#endif
    }
  }

  void connect(std::string ip, uint16_t port) 
  {
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    _remoteEndPoint.address(address);
    _remoteEndPoint.port(port);
  }

  void send(const char *data, uint32_t size) 
  {
    _socket.async_send_to(boost::asio::buffer(data, size), _remoteEndPoint,
      boost::bind(&UDPBcastClientSocket::processDataSentEvent, this,
      boost::asio::placeholders::error));
  }

  void send(const std::string data) 
  {
    _socket.async_send_to(boost::asio::buffer(data.c_str(), data.size()), _remoteEndPoint,
      boost::bind(&UDPBcastClientSocket::processDataSentEvent, this,
      boost::asio::placeholders::error));
  }

  void processDataSentEvent(const boost::system::error_code& error)
  {
  }
};

ClientSocket::ClientSocket(Net& net):
  Socket(net)
{}

ClientSocket::~ClientSocket()
{}

ServerSocket * 
ServerSocket::Create(Net& net, SocketOptions option, ...)
{
  std::string txt;
  int reuseAddr = SAI_SOCKET_OPT_FALSE;
  int protocol  = SAI_SOCKET_OPT_UDP;
  int useIpMulticast = SAI_SOCKET_OPT_FALSE;
  ServerSocket * ret = 0;

  va_list pa;
  va_start(pa, option);
  for (; option != SAI_SOCKET_EOA; option = (SocketOptions) va_arg(pa, int))
  {
    switch(option)
    {
      case SAI_SOCKET_B_REUSE_ADDR:
        reuseAddr = va_arg(pa, int); 
        break;
      case SAI_SOCKET_I_PROTOCOL:
        protocol = va_arg(pa, int); 
        break;
      case SAI_SOCKET_B_USEIP_MULTICAST:
        useIpMulticast = va_arg(pa, int);
        break;
      default:
        txt = "Invalid option for ServerSocket!";
        break;
    }
  }
  va_end(pa);

  if (txt.length() == 0)
  {
    switch (protocol)
    {
      case SAI_SOCKET_OPT_UDP:
        if (useIpMulticast)
        { 
          ret = new UDPMcastServerSocket(net);
          if (reuseAddr) 
          {
            (dynamic_cast<UDPMcastServerSocket*>(ret))->reuseAddr = true;
          }
        }
        else
        {
          txt = "Currently, UDP/UNICAST does not support!";
        }
        break;
      case SAI_SOCKET_OPT_TCP:
        if (!useIpMulticast)
        {
          ret = new TCPServerSocket(net);
          if (reuseAddr)
          {
            (dynamic_cast<TCPServerSocket*>(ret))->reuseAddr = true;
          }
        }
        else
        {
          txt = "Invalid option for TCP socket is specified!";
        }
        break;
    }
  }

  if (txt.length() > 0)
  {
    throw SocketException(txt);
  }

  return ret;
}

ClientSocket * 
ClientSocket::Create(Net& net, SocketOptions option, ...)
{
  std::string txt;
  int protocol  = SAI_SOCKET_OPT_UDP;
  int useIpMulticast = SAI_SOCKET_OPT_FALSE;
  int broadcast = SAI_SOCKET_OPT_FALSE;
  ClientSocket * ret = 0;

  va_list pa;
  va_start(pa, option);
  for (; option != SAI_SOCKET_EOA; option = (SocketOptions) va_arg(pa, int))
  {
    switch(option)
    {
      case SAI_SOCKET_I_PROTOCOL:
        protocol = va_arg(pa, int); 
        break;
      case SAI_SOCKET_B_USEIP_MULTICAST:
        useIpMulticast = va_arg(pa, int);
        break;
      case SAI_SOCKET_B_BROADCAST:
        broadcast = va_arg(pa, int);
        break;
      default:
        txt = "Invalid option for ServerSocket!";
        break;
    }
  }
  va_end(pa);

  if (txt.length() == 0)
  {
    switch (protocol)
    {
      case SAI_SOCKET_OPT_UDP:
        if (useIpMulticast)
        { 
          ret = new UDPMcastClientSocket(net);
        }
        else if (broadcast)
        {
          ret = new UDPBcastClientSocket(net);
        }
        else
        {
          txt = "Currently, UDP/UNICAST does not support!";
        }
        break;
      case SAI_SOCKET_OPT_TCP:
        if (!useIpMulticast)
        {
          ret = new TCPClientSocket(net);
        }
        else
        {
          txt = "Invalid option for TCP socket is specified!";
        }
        break;
    }
  }

  if (txt.length() > 0)
  {
    throw SocketException(txt);
  }

  return ret;
}

Socket::Socket(Net& net):
  _net(net)
{}

Socket::~Socket()
{}
