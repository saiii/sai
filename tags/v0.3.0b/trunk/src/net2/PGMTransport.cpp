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

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <syslog.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <iostream>

#include <utils/Logger.h>

#include <net2/Net.h>
#include <net2/Nic.h>
#include <net2/NicList.h>
#include <net2/PGMSocket.h>
#include "Transport.h"

#define NET_BUFFER_SIZE 8192

namespace sai { 
namespace net2 {

class PgmTransportImpl
{
public:
  PGMSocket * sckt;

public:
  PgmTransportImpl():
    sckt(0)
  {
  }

  ~PgmTransportImpl()
  {
  }

  void initialize(NetworkOptions * options)
  {
    sckt = PGMSocketFactory::GetInstance()->create(options);
  }

  void send(const char *data, uint32_t size)
  {
#if DEBUG_NET
    static uint16_t seqNo = 54300000;
    seqNo++;
    printf("Seq %u, Size %u\n", seqNo, size);
    uint16_t nSeqNo = htons(seqNo);
    memcpy((char*)(data + 10), &nSeqNo, sizeof(nSeqNo));
#endif

    sckt->send(data, size);
  }

  void send(const std::string data)
  {
  }
};

}}

using namespace sai::net2;

PgmTransport::PgmTransport():
  _impl(0)
{
  _impl = new PgmTransportImpl();
}

PgmTransport::~PgmTransport()
{
  delete _impl;
}

void 
PgmTransport::bind(std::string ip)
{
}

void 
PgmTransport::close()
{
}

void 
PgmTransport::destination(std::string ip, uint16_t port)
{
}

void
PgmTransport::send(const char * data, uint32_t size)
{
  _impl->send(data, size);
}

void
PgmTransport::send(std::string data)
{
  _impl->send(data);
}

void 
PgmTransport::setOptions(NetworkOptions* options)
{
  _impl->initialize(options);
}

