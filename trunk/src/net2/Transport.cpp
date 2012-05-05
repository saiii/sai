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

#ifndef _WIN32
#include <syslog.h>
#endif

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <utils/Logger.h>
#include <net2/DataHandler.h>
#include <net2/DataDescriptor.h>

#include "Net.h"
#include "Transport.h"

#define NET_BUFFER_SIZE 8192

using namespace sai::net2;

Transport::Transport()
{
}

Transport::~Transport()
{
}

void 
Transport::bind(std::string ip)
{
}

namespace sai
{
namespace net2
{

class TcpTransportImpl
{
public:
  boost::asio::ip::tcp::socket   sckt;
  TcpTransportEventHandler     * handler;
  char                         * buffer;

public:
  TcpTransportImpl():
    sckt(*((boost::asio::io_service*)Net::GetInstance()->getIO())),
    handler(0)
  {
    buffer = new char[NET_BUFFER_SIZE];
  }

  ~TcpTransportImpl()
  {
    delete [] buffer;
  }

  void connect(std::string ip, uint16_t port)
  {
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    boost::asio::ip::tcp::endpoint endpoint(address, port);

    sckt.async_connect(endpoint,
          boost::bind(&TcpTransportImpl::processConnectedEvent, this, boost::asio::placeholders::error));
  }

  void close()
  {
    try
    {
#ifndef _WIN32
      sckt.cancel();
#endif
      sckt.shutdown(boost::asio::socket_base::shutdown_both);
      sckt.close();

      if (handler) handler->processConnectionClosedEvent();
    }
    catch(boost::system::system_error& e)
    {
#ifndef _WIN32
      openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
      syslog(LOG_ERR, "Failed to close a TcpTransportImpl");
      syslog(LOG_ERR, "%s", e.what());
      closelog();
#endif
    }
  }

  void send(const char * data, uint32_t size)
  {
    std::string message;
    message.append(data, size);
    boost::system::error_code ignored_error;
    boost::asio::write(sckt, boost::asio::buffer(message), ignored_error);
  }
 
  void send(std::string message)
  {
    boost::system::error_code ignored_error;
    boost::asio::write(sckt, boost::asio::buffer(message), ignored_error);
  }

  void readPreparation()
  {
    sckt.async_read_some(
      boost::asio::buffer(buffer, NET_BUFFER_SIZE),
      boost::bind(&TcpTransportImpl::processDataEvent, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }

  void processDataEvent(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err)
    {
      if (handler) 
      {
        handler->processDataEvent(buffer, bytes_transferred);
      }
      readPreparation();
    }
    else
    {
      if (handler) 
      {
        handler->processDataEvent(0, -1);
      }
    }
  }

  void processConnectedEvent(const boost::system::error_code& err)
  {
    if (!err)
    {
      readPreparation();
    }
  }
};

}}


TcpTransport::TcpTransport():
  _impl(0)
{
  _impl = new TcpTransportImpl();
}

TcpTransport::~TcpTransport()
{
  delete _impl;
}

void 
TcpTransport::readPreparation()
{
}

void 
TcpTransport::connect(std::string ip, uint16_t port)
{
  _impl->connect(ip, port);
}

void 
TcpTransport::close()
{
  _impl->close();
}

void
TcpTransport::send(const char * data, uint32_t size)
{
  _impl->send(data, size);
}

void
TcpTransport::send(std::string data)
{
  _impl->send(data);
}

void 
TcpTransport::setHandler(TcpTransportEventHandler * handler)
{
  _impl->handler = handler;
}

namespace sai { 
namespace net2 {

class UdpTransportImpl
{
public:
  boost::asio::ip::udp::endpoint  remoteEndPoint;
  boost::asio::ip::udp::socket    sckt;

public:
  UdpTransportImpl():
    sckt(*((boost::asio::io_service*)Net::GetInstance()->getIO()), remoteEndPoint.protocol())
  {
    boost::asio::ip::udp::endpoint endp;
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(Net::GetInstance()->getLocalAddress());
    endp.address(address);
    sckt.bind(endp);

    if (sai::utils::Logger::GetInstance())
    {
      sai::utils::Logger::GetInstance()->print(
        sai::utils::Logger::SAILVL_DEBUG, "McastTransportImpl bind %s\n", Net::GetInstance()->getLocalAddress().c_str());
    }
  }

  ~UdpTransportImpl()
  {}

  void close()
  {
    try
    {
#ifndef _WIN32
      sckt.cancel();
#endif
      sckt.shutdown(boost::asio::socket_base::shutdown_both);
      sckt.close();
      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_DEBUG, "McastTransportImpl closed\n");
      }
    }
    catch(boost::system::system_error& e)
    {
      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_ERROR, "McastTransportImpl failed to close\n");
      }
#ifdef _WIN32
#else
      openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
      syslog(LOG_ERR, "Failed to close a McastTransportImpl");
      syslog(LOG_ERR, "%s", e.what());
      closelog();
#endif
    }
  }

  void bind(std::string ip)
  {
    boost::asio::ip::udp::endpoint endp;
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    endp.address(address);
    sckt.bind(endp);
  }

  void destination(std::string ip, uint16_t port)
  {
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    remoteEndPoint.address(address);
    remoteEndPoint.port(port);
  }

  void send(const char *data, uint32_t size)
  {
    sckt.send_to(boost::asio::buffer(data, size), remoteEndPoint);
  }

  void send(const std::string data)
  {
    sckt.send_to(boost::asio::buffer(data.c_str(), data.size()), remoteEndPoint);
  }
};

}}


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
  _impl->close();
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

namespace sai
{
namespace net2
{

class Session
{
public:
  char*                         buffer;
  boost::asio::ip::tcp::socket  sckt;
  DataHandler*                  handler; 

public:
  Session():
    buffer(0),
    sckt(*((boost::asio::io_service*)Net::GetInstance()->getIO())),
    handler(0)
  {
    buffer = new char[NET_BUFFER_SIZE];
  }
  ~Session()
  {
    delete [] buffer;
  }
  void start()
  {
    sckt.async_read_some(boost::asio::buffer(buffer, NET_BUFFER_SIZE),
      boost::bind(&Session::processDataEvent, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }
  void processDataEvent(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err)
    {
      if (handler) 
      {
        DataDescriptor desc;
        // do something with buffer and bytes_transferred
        handler->processDataEvent(desc);
      }
      start();
    }
    else
    {
      delete this;
    }
  }
};

class InternalTransportImpl
{
public:
  // TCP Stuff
  boost::asio::ip::tcp::acceptor* acceptor;
  DataHandler*                    handler; 

  // UDP Stuff
  boost::asio::ip::udp::socket   udpSocket;
  boost::asio::ip::udp::endpoint udpSenderEndpoint;
  char                          *udpBuffer;

public:
  InternalTransportImpl():
    acceptor(0),
    handler(0),
    udpSocket(*((boost::asio::io_service*)Net::GetInstance()->getIO()))
  {
    udpBuffer = new char[NET_BUFFER_SIZE];
  }

  ~InternalTransportImpl()
  {
    delete [] udpBuffer;
  }

  void initialize(std::string ip, uint16_t port, DataHandler * hndlr)
  {
    handler = hndlr;

    // TCP Stuff
    {
      boost::asio::ip::tcp::endpoint endpoint;
      const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
      endpoint.address(address);
      endpoint.port(port);

      acceptor = new boost::asio::ip::tcp::acceptor(
        *((boost::asio::io_service*)Net::GetInstance()->getIO()), endpoint);
      acceptor->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
  
      Session * session = new Session();
      acceptor->async_accept(session->sckt,
        boost::bind(&InternalTransportImpl::processConnectionRequestEvent,
                    this, session, boost::asio::placeholders::error));
    }

    // UDP Stuff
    {
      boost::asio::ip::udp::endpoint endpoint;
      const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
      endpoint.address(address);
      endpoint.port(port);

      // open
      udpSocket.open(endpoint.protocol());
      udpSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

      // bind
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(endpoint.port());
      //addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_addr.s_addr = endpoint.address().to_string().compare("0.0.0.0") == 0 ?
                             htonl(INADDR_ANY) : inet_addr(endpoint.address().to_string().c_str());

      if (::bind(udpSocket.native(),(struct sockaddr *)&addr, sizeof(addr)) < 0)
      {
        udpSocket.close();
        return;
      }

      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_DEBUG, "InternalTransportImpl bind %s\n", endpoint.address().to_string().c_str());
      }

      // join (default channel)
      std::string mcast = "224.250.250.250";
      struct ip_mreq imreq;
      memset(&imreq, 0, sizeof(struct ip_mreq));

      imreq.imr_multiaddr.s_addr = inet_addr(mcast.c_str());
      imreq.imr_interface.s_addr = inet_addr(endpoint.address().to_string().c_str());
      //imreq.imr_interface.s_addr = INADDR_ANY;

      int ret = setsockopt(udpSocket.native(), IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&imreq, sizeof(struct ip_mreq));
      if (ret != 0)
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_ERROR, "Unable to join the specified multicast group\n");
      }

      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_INFO, "InternalTransportImpl joins mcast group (%s) on the (%s)\n",
          mcast.c_str(), endpoint.address().to_string().c_str());
      }

      // listen
      udpSocket.async_receive_from(
        boost::asio::buffer(udpBuffer, NET_BUFFER_SIZE), udpSenderEndpoint,
        boost::bind(&InternalTransportImpl::processDataEvent, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
  }

  void processConnectionRequestEvent(Session * session, const boost::system::error_code& err) // TCP Only
  {
    if (!err)
    {
      session->start();
    }
    else
    {
      delete session;
    }

    Session * sess = new Session();
    acceptor->async_accept(sess->sckt,
      boost::bind(&InternalTransportImpl::processConnectionRequestEvent,
                    this, sess, boost::asio::placeholders::error));
  }

  void processDataEvent(const boost::system::error_code& error, size_t bytes_recvd) // UDP Only
  {
    if (error)
    {
      return;
    }

    if (handler)
    {
      DataDescriptor desc;
      // Do something with udpBuffer, bytes_recvd
      handler->processDataEvent(desc);
    }

    udpSocket.async_receive_from(
      boost::asio::buffer(udpBuffer, NET_BUFFER_SIZE), udpSenderEndpoint,
      boost::bind(&InternalTransportImpl::processDataEvent, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }


  void close()
  {
    // TCP Stuff
    {
      try
      {
        acceptor->cancel();
        acceptor->close();
        delete acceptor;
        acceptor = 0;
      } 
      catch(boost::system::system_error& e)
      {
#ifndef _WIN32
        openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        syslog(LOG_ERR, "Failed to close InternalTransportImpl");
        syslog(LOG_ERR, "%s", e.what());
        closelog();
#endif
      }
    }

    // UDP Stuff
    {
      try
      {
#ifndef _WIN32
        udpSocket.cancel();
#endif
        udpSocket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
        udpSocket.close();
        if (sai::utils::Logger::GetInstance())
        {
          sai::utils::Logger::GetInstance()->print(
            sai::utils::Logger::SAILVL_DEBUG, "InternalTransportImpl closed\n");
        }
      } 
      catch(boost::system::system_error& e)
      {
        if (sai::utils::Logger::GetInstance())
        {
          sai::utils::Logger::GetInstance()->print(
            sai::utils::Logger::SAILVL_ERROR, "InternalTransportImpl failed to close\n");
        }
#ifndef _WIN32
        openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        syslog(LOG_ERR, "Failed to close a InternalTransportImpl");
        syslog(LOG_ERR, "%s", e.what());
        closelog();
#endif
      }
    }
  }
};

}}

InternalTransport::InternalTransport():
  _impl(0),
  _handler(0)
{
  _impl = new InternalTransportImpl();
}

InternalTransport::~InternalTransport()
{
  _impl->close();
  delete _impl;
}

void 
InternalTransport::initialize(std::string ip, uint16_t port, DataHandler * handler)
{
  _impl->initialize(ip, port, handler);
}

