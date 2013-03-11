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

#ifndef __SAI_NET_PGMSOCKET__
#define __SAI_NET_PGMSOCKET__

#include <string>
#include <utils/ThreadPool.h>
#include <pgm/pgm.h>

namespace sai
{
namespace net
{

class PGMSocketEventHandler
{
public:
  virtual void processDataEvent(char *, uint32_t) {}
};

class NetworkOptions
{
private:
  uint16_t            _port;
  std::string         _interface;
  std::string         _send;
  std::vector<char *> _receive;
  bool                _withHttp;

public:
  NetworkOptions();
  ~NetworkOptions();

  void     setInterface(std::string intf);
  void     addReceive(std::string rec);
  void     setSend(std::string snd);
  void     setPort(uint16_t port) { _port = port; }
  void     setHttp(bool v)        { _withHttp = v;}
  uint16_t getPort()              { return _port; }
  bool     enableHttp()           { return _withHttp; }
  void     toString(std::string& ret);
};

class Receiver : public sai::utils::ThreadTask
{
private:
  pgm_sock_t*             _sckt;
  PGMSocketEventHandler * _handler;
  bool                    _running;

public:
  Receiver(pgm_sock_t* sckt, PGMSocketEventHandler *);
  ~Receiver();

  void threadEvent();
  void setEnable(bool v) { _running = v; }
};

class PGMSocket 
{
private:
  static uint8_t          _InitCnt;
  PGMSocketEventHandler * _handler;

  pgm_sock_t*             _scktRecv;
  pgm_sock_t*             _scktSend;
  struct pgm_addrinfo_t * _addrInfo;
  
  Receiver *              _receiver;

public:
  PGMSocket(NetworkOptions* options);
  ~PGMSocket();
  void listen();
  void setEventHandler(PGMSocketEventHandler * h);

  void send(const char *data, uint32_t size);
  void close();
};

}}

#endif
