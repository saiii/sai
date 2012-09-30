//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
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

#ifndef __SAI_NET2_TRANSPORT__
#define __SAI_NET2_TRANSPORT__

#include <stdint.h>
#include <string>
#include <net2/DataHandler.h>

namespace sai
{
namespace net2
{

class Transport
{
public:
  Transport();
  virtual ~Transport();

  virtual void bind(std::string ip);
  virtual void open();
  virtual void close() = 0;
  virtual void send(const char * data, uint32_t size) = 0;
  virtual void send(std::string data) = 0;
};

class TcpTransportEventHandler : public RawDataHandler
{
public:
  virtual void processConnectionClosedEvent() {}
  virtual void processDataEvent(const char * buffer, const uint32_t bytes) {}
};

class TcpTransportImpl;
class TcpTransport : public Transport
{
private:
  TcpTransportImpl * _impl;

public:
  TcpTransport();
  virtual ~TcpTransport();

  void connect(std::string ip, uint16_t port);
  void close();
  void send(const char * data, uint32_t size);
  void send(std::string data);
  void setHandler(TcpTransportEventHandler * handler);
};

class UdpTransportImpl;
class UdpTransport : public Transport
{
private:
  UdpTransportImpl * _impl;

public:
  UdpTransport();
  virtual ~UdpTransport();

  void bind(std::string ip);
  void close();
  void send(const char * data, uint32_t size);
  void send(std::string data);
  void destination(std::string ip, uint16_t port);
};

class McastSetImpl;
class InternalTransportImpl;
class McastSet
{
friend class InternalTransportImpl;
private:
  McastSetImpl* _impl;

public:
  McastSet();
  virtual ~McastSet();

  void add(std::string mcast);
};

class InternalTransport
{
friend class DataMessengerFactory;
private:
  InternalTransportImpl * _impl;
  RawDataHandler *        _handler;
  McastSet *              _mcastSet;

private:
  InternalTransport();

public:
  void initialize(std::string ip, uint16_t port, McastSet* mcastSet, RawDataHandler * handler);
  ~InternalTransport();
};

}}

#endif
