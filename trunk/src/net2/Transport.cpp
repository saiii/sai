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

#include <net2/Net.h>
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

void 
Transport::open()
{
}

namespace sai
{
namespace net2
{

typedef std::vector<std::string>           StringList;
typedef std::vector<std::string>::iterator StringListIterator;

class McastSetImpl
{
public:
  StringList list;

public:
  McastSetImpl()
  {}

  ~McastSetImpl()
  {}

  void add(std::string mcast)
  {
    list.push_back(mcast);
  }
};

class Session
{
public:
  char*                         buffer;
  boost::asio::ip::tcp::socket  sckt;
  RawDataHandler*               handler; 

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
        // FIXME : How to send data back in this stream before it gets closed
        handler->processDataEvent(buffer, bytes_transferred);
      }
      start();
    }
    else
    {
      // FIXME : We may need to start listening again
      delete this;
    }
  }
};

class InternalTransportImpl
{
public:
  // TCP Stuff
  boost::asio::ip::tcp::acceptor* acceptor;
  RawDataHandler*                 handler; 

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

  void initialize(std::string ip, uint16_t port, McastSet* mcastSet, RawDataHandler * hndlr)
  {
    handler = hndlr;

    // TCP Stuff
    try{
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
    catch(boost::system::system_error& e) {}

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
      McastSetImpl * set = mcastSet->_impl;
      for (uint32_t i = 0; i < set->list.size(); i += 1)
      {
        std::string mcast = set->list.at(i);
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
      session->handler = handler;
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
      // SAI
      printf("Receive UDP packet from %s, port = %u\n", 
        udpSenderEndpoint.address().to_string().c_str(),
        udpSenderEndpoint.port());
      handler->processDataEvent(udpBuffer, bytes_recvd);
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
InternalTransport::initialize(std::string ip, uint16_t port, McastSet* mcastSet, RawDataHandler * handler)
{
  _impl->initialize(ip, port, mcastSet, handler);
}

McastSet::McastSet():
  _impl(0)
{
  _impl = new McastSetImpl();
}

McastSet::~McastSet()
{
  delete _impl;
}

void 
McastSet::add(std::string mcast)
{
  _impl->add(mcast);
}

