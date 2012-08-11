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
#include <net2/Nic.h>
#include <net2/NicList.h>
#include "Transport.h"

#define NET_BUFFER_SIZE 8192

using namespace sai::net2;

namespace sai { 
namespace net2 {

class UdpTransportImpl
{
public:
  boost::asio::ip::udp::endpoint  myendp;
  boost::asio::ip::udp::endpoint  remoteEndPoint;
  boost::asio::ip::udp::socket    sckt;

public:
  UdpTransportImpl():
    sckt(*((boost::asio::io_service*)Net::GetInstance()->getIO()), remoteEndPoint.protocol())
  {
    //Net::GetInstance()->getNicList()->getDefaultNic()->toString(s2->ipaddr);
  }

  ~UdpTransportImpl()
  {
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
      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_DEBUG, "UdpTransportImpl closed\n");
      }
    }
    catch(boost::system::system_error& e)
    {
      if (sai::utils::Logger::GetInstance())
      {
        sai::utils::Logger::GetInstance()->print(
          sai::utils::Logger::SAILVL_ERROR, "UdpTransportImpl failed to close\n");
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
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    myendp.address(address);
    sckt.bind(myendp);

    if (sai::utils::Logger::GetInstance())
    {
      sai::utils::Logger::GetInstance()->print(
        sai::utils::Logger::SAILVL_DEBUG, "UdpTransportImpl bind %s\n", ip.c_str());
    }
  }

  void destination(std::string ip, uint16_t port)
  {
    const boost::asio::ip::address address = boost::asio::ip::address::from_string(ip);
    remoteEndPoint.address(address);
    remoteEndPoint.port(port);
  }

  void send(const char *data, uint32_t size)
  {
    bool error = false;
    size_t bytes = 0;
    try
    {
      bytes = sckt.send_to(boost::asio::buffer(data, size), remoteEndPoint);
    }
    catch(boost::system::system_error& e)
    {
      error = true;
    }

    if (error || bytes != size)
    {
      //fprintf(stderr, "Sending error!\n");
    }
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

