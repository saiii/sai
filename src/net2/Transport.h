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
#include <vector>

namespace sai
{
namespace net2
{

typedef enum
{
  UDP,
  PGM,
  TCP,
  ZMQ
}IncomingSource;

class TCPSessionDestroyHandler
{
public:
  virtual void destroyEvent() = 0;
};

class DirectReply
{
protected:
  TCPSessionDestroyHandler * _dhandler;

public:
  DirectReply():_dhandler(0) {}
  ~DirectReply() { _dhandler = 0; }

  virtual void send(char * data, int size) = 0;
  void setDestroyHandler(TCPSessionDestroyHandler * handler) { _dhandler = handler; }
};

typedef std::vector<DirectReply*>           DirectReplyList;
typedef std::vector<DirectReply*>::iterator DirectReplyListIterator;

class DirectDb
{
private:
  DirectReplyList _list;

public:
  DirectDb(); 
  ~DirectDb(); 
  void add(DirectReply*);
  void drop(DirectReply*);
  bool has(DirectReply*);
};

class TCPPtr
{
public:
  DirectDb *    db;
  DirectReply * obj;

public:
  TCPPtr(DirectDb * d, DirectReply * r) : db(d), obj(r) {}
};

class Endpoint
{
private:
  std::string    _addr;
  uint16_t       _port;
  IncomingSource _source;
  TCPPtr         _tcpPtr;

public:
  Endpoint(std::string addr, uint32_t p, IncomingSource source, TCPPtr* ptr);
  ~Endpoint();

  std::string&   address()  { return _addr; }
  uint16_t       port()     { return _port; }
  IncomingSource source()   { return _source; }
  TCPPtr*        getTCPPtr(){ return &_tcpPtr;}
};

class NetworkOptions
{
private:
  uint16_t            _port;
  std::string         _interface;
  std::string         _send;
  std::vector<char *> _receive;
  bool                _withHttp;
  bool                _disablePGM;
  bool                _enableSSL;

public:
  NetworkOptions();
  ~NetworkOptions();

  void     setInterface(std::string intf);
  void     addReceive(std::string rec);
  void     setSend(std::string snd);
  void     setPort(uint16_t port) { _port = port; }
  void     setHttp(bool v)        { _withHttp = v;}
  void     setDisablePGM()        { _disablePGM = true; }
  void     setEnableTCP()         { _disablePGM = true; }
  void     setEnableSSL(bool v)   { _enableSSL = v;     }
  uint16_t getPort()              { return _port; }
  bool     enableHttp()           { return _withHttp;  }
  bool     enableSSL()            { return _enableSSL; }
  void     toString(std::string& ret);
  void     copy(NetworkOptions* opt);
  bool     equals(NetworkOptions* opt);
  bool     usePGM()               { return !_disablePGM; };
  std::string& getInterface()     { return _interface;   };
};

class PerfMeasure
{
public:
  static uint32_t IncomingTime;
  static uint32_t OutgoingTime;
  static uint32_t InCount;
  static uint32_t OutCount;
};

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
  virtual void processDataEvent(Endpoint* endpoint, const char * buffer, const uint32_t bytes) {}
};

class TcpTransportImpl;
class TcpTransport : public Transport
{
private:
  TcpTransportImpl * _impl;

public:
  TcpTransport();
  TcpTransport(bool ssl);
  virtual ~TcpTransport();

  virtual void connect(std::string ip, uint16_t port);
  virtual void close();
  virtual void send(const char * data, uint32_t size);
  virtual void send(std::string data);
  virtual void setHandler(TcpTransportEventHandler * handler);
  virtual bool isConnected();
  virtual Endpoint * getLocalEndpoint();
  virtual void setTimeout(int msec);
};

class PgmTransportImpl;
class PGMSocket;
class PgmTransport : public Transport
{
private:
  PgmTransportImpl * _impl;

public:
  PgmTransport();
  virtual ~PgmTransport();

  virtual void bind(std::string ip);
  virtual void close();
  virtual void send(const char * data, uint32_t size);
  virtual void send(std::string data);
  virtual void destination(std::string ip, uint16_t port);

  virtual void setOptions(NetworkOptions* options);
};

class UdpSocket
{
friend class UdpTransportImpl;
friend class InternalTransportImpl;
friend class TransportThread;
private:
  static UdpSocket*  _instance;
  int                _iSendFd;
  int                _lSendFd;

private:
  UdpSocket();
  ~UdpSocket();
  void init();
};

class UdpTransportImpl;
class UdpTransport : public Transport
{
private:
  UdpTransportImpl * _impl;

public:
  UdpTransport();
  virtual ~UdpTransport();

  virtual void bind(std::string ip);
  virtual void close();
  virtual void send(const char * data, uint32_t size);
  virtual void send(std::string data);
  virtual void destination(std::string ip, uint16_t port);
};

class InternalTransportImpl;
class InternalTransport
{
friend class DataMessengerFactory;
private:
  InternalTransportImpl * _impl;
  RawDataHandler *        _handler;

private:
  InternalTransport();

public:
  void initialize(NetworkOptions* opt, RawDataHandler * handler);
  ~InternalTransport();
  PgmTransport * getPgmTransport();
};

}}

#endif
