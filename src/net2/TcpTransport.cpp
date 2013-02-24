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
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

#include <utils/Logger.h>

#include <net2/Net.h>
#include "Transport.h"

#define NET_BUFFER_SIZE1  40960
#define NET_BUFFER_SIZE2 819200

using namespace sai::net2;

namespace sai
{
namespace net2
{

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

class TcpTransportImpl
{
private:
  std::string _ip;
  uint16_t    _port;

public:
  boost::asio::ip::tcp::socket * sckt;
  ssl_socket                   * sslSckt;
  TcpTransportEventHandler     * handler;
  char                         * buffer;
  Endpoint                     * localEndpoint;
  bool                           connected;

public:
  TcpTransportImpl(boost::asio::ssl::context* context):
    _port(0),
    sckt(0),
    sslSckt(0),
    handler(0), 
	localEndpoint(0),
    connected(false)
  {
    if (!context)
    {
      sckt = new boost::asio::ip::tcp::socket(*((boost::asio::io_service*)Net::GetInstance()->getIO()));
    }
    else
    {
      sslSckt = new ssl_socket(*((boost::asio::io_service*)Net::GetInstance()->getIO()), *context);
      sslSckt->set_verify_mode(boost::asio::ssl::verify_peer);
      sslSckt->set_verify_callback(boost::bind(&TcpTransportImpl::verifyCertificate, this, _1, _2));

    }
    buffer = new char[NET_BUFFER_SIZE2];
  }

  ~TcpTransportImpl()
  {
    delete localEndpoint;
    delete [] buffer;
  }

  bool verifyCertificate(bool preverified, boost::asio::ssl::verify_context& ctx)
  {
    return preverified;
  }

  ssl_socket::lowest_layer_type& sockt()
  {
    return sslSckt->lowest_layer();
  }
 
  void connect(std::string ip, uint16_t port)
  {
    _ip   = ip;
    _port = port;    
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    boost::asio::ip::tcp::endpoint endpoint(address, port);

    char sPort[16];
    sprintf(sPort, "%u", port);
    try
    {
      boost::asio::ip::tcp::resolver resolver(*((boost::asio::io_service*)Net::GetInstance()->getIO()));
      boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip, sPort);
      boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
      boost::asio::connect(sckt ? *sckt : sockt(), iterator);

      connected = true;
      boost::asio::ip::tcp::no_delay option(true);
      if (sckt)
      {
        sckt->set_option(option);
      }
      else
      {
        sockt().set_option(option);
      }

      boost::asio::ip::tcp::socket::send_buffer_size option2(0);
      if (sckt)
      {
        sckt->set_option(option2);
      }
      else
      {
        sockt().set_option(option2);
      }

      readPreparation();
    }
    catch(boost::system::system_error& e)
    {
    }
    //sckt.async_connect(endpoint,
    //      boost::bind(&TcpTransportImpl::processConnectedEvent, this, boost::asio::placeholders::error));
  }

  void setTimeout(int msec)
  {
#ifdef _WIN32
	DWORD timeout = msec;
    setsockopt(sockt().native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sockt().native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#endif
  }

  void close()
  {
    try
    {
      connected = false;
      if (sckt)
      {
#ifndef _WIN32
        sckt->cancel();
#endif
        sckt->shutdown(boost::asio::socket_base::shutdown_both);
        sckt->close();
      }
      else
      {
#ifndef _WIN32
        sockt().cancel();
#endif
        sockt().shutdown(boost::asio::socket_base::shutdown_both);
        sockt().close();
      }

      if (handler) 
      {
        handler->processConnectionClosedEvent();
        handler = 0;
      }
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
    boost::system::error_code ignored_error;

    bool isOpen = sckt ? sckt->is_open() : sockt().is_open();
    if (!isOpen) 
    {
      const boost::asio::ip::address address = boost::asio::ip::address::from_string(_ip);
      boost::asio::ip::tcp::endpoint endpoint(address, _port);
      if (sckt)
      {
        sckt->async_connect(endpoint, boost::bind(&TcpTransportImpl::processConnectedEvent, this, boost::asio::placeholders::error));
      }
      else
      {
        sockt().async_connect(endpoint, boost::bind(&TcpTransportImpl::processConnectedEvent, this, boost::asio::placeholders::error));
      }
      return;
    }

#if DEBUG_NET
    static uint16_t seqNo = 0;
    seqNo++;
    printf("Seq %u, Size %u\n", seqNo, size);
    uint16_t nSeqNo = htons(seqNo);
    memcpy((char*)(data + 10), &nSeqNo, sizeof(nSeqNo));
#endif

    message.append(data, size);

    if (sckt)
    {
      int32_t bytes = boost::asio::write(*sckt, boost::asio::buffer(message), ignored_error);
      if (bytes != message.size())
      {
        if (bytes == 0)
        {
          close();
        }
      }
    }
    else
    {
      char * nmsg = new char[size];
      DataSentHandler * handler = new DataSentHandler();
      handler->msg  = nmsg;
      handler->impl = this;
      memcpy(nmsg, data, size);
      boost::asio::async_write(*sslSckt, boost::asio::buffer(nmsg, size),
          boost::bind(&TcpTransportImpl::DataSentHandler::processDataSent, handler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  class DataSentHandler 
  {
    public:
      TcpTransportImpl * impl;
      char             * msg;

    public:
      void processDataSent(const boost::system::error_code& error, size_t bytes_transferred)
      {
        if (error)
        {
          impl->close();
        }
        delete [] msg;
        delete this;
      }
  };
 
  void send(std::string message)
  {
    boost::system::error_code ignored_error;
    if (sckt)
    {
      boost::asio::write(*sckt, boost::asio::buffer(message), ignored_error);
    }
    else
    {
      char * nmsg = new char[message.size()];
      DataSentHandler * handler = new DataSentHandler();
      handler->msg  = nmsg;
      handler->impl = this;
      memcpy(nmsg, message.data(), message.size());
      boost::asio::async_write(*sslSckt, boost::asio::buffer(nmsg, message.size()),
          boost::bind(&TcpTransportImpl::DataSentHandler::processDataSent, handler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  void readPreparation()
  {
    if (sckt)
    {
      sckt->async_read_some(
        boost::asio::buffer(buffer, NET_BUFFER_SIZE1),
        boost::bind(&TcpTransportImpl::processDataEvent, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    }
    else 
    {
      boost::asio::async_read(*sslSckt,
          boost::asio::buffer(buffer, NET_BUFFER_SIZE1),
          boost::bind(&TcpTransportImpl::processDataEvent, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  Endpoint * getLocalEndpoint()
  {
	  return localEndpoint;
  }

  void processDataEvent(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err)
    {
      if (handler) 
      {
        boost::asio::ip::tcp::endpoint ep = sckt ? sckt->remote_endpoint() : sockt().remote_endpoint();
        Endpoint endpoint(ep.address().to_string(), ep.port(), TCP, 0);

		boost::asio::ip::tcp::endpoint ep2 = sckt ? sckt->local_endpoint() : sockt().local_endpoint();
		std::string a = ep2.address().to_string();
		if (localEndpoint)
		{
			delete localEndpoint;
		}
		localEndpoint = new Endpoint(a, ep2.port(), TCP, 0);
        handler->processDataEvent(&endpoint, buffer, bytes_transferred);
      }
      readPreparation();
    }
    else
    {
      if (handler) 
      {
        handler->processConnectionClosedEvent();
        handler = 0;
      }
    }
  }

  void processConnectedEvent(const boost::system::error_code& err)
  {
    if (!err)
    {
      connected = true;
      boost::asio::ip::tcp::no_delay option(true);
      if (sckt)
      {
        sckt->set_option(option);
      }
      else
      {
        sockt().set_option(option);
      }

      boost::asio::ip::tcp::socket::send_buffer_size option2(0);
      if (sckt)
      {
        sckt->set_option(option2);
      }
      else
      {
        sockt().set_option(option2);
      }
      readPreparation();
    }
  }
};

}}


TcpTransport::TcpTransport():
  _impl(0)
{
  _impl = new TcpTransportImpl(0);
}

TcpTransport::TcpTransport(bool ssl):
  _impl(0)
{
  boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
  ctx.load_verify_file("ca.pem");

  _impl = new TcpTransportImpl(&ctx);
}

TcpTransport::~TcpTransport()
{
  delete _impl;
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

bool
TcpTransport::isConnected()
{
  return _impl->connected;
}

void 
TcpTransport::setTimeout(int msec)
{
  _impl->setTimeout(msec);
}

Endpoint * 
TcpTransport::getLocalEndpoint()
{
	return _impl->getLocalEndpoint();
}