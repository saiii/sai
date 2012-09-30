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

