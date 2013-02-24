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

#ifndef __SAI_NET2_PGMSOCKET__
#define __SAI_NET2_PGMSOCKET__

#include <string>
#include <vector>
#include <utils/ThreadPool.h>
#include <net2/DataHandler.h>
#include <net2/Transport.h>
#include <pgm/pgm.h>

namespace sai
{
namespace net2
{

class Receiver;
class PGMSocket 
{
private:
  static uint8_t          _InitCnt;
  RawDataHandler *        _handler;

  pgm_sock_t*             _scktRecv;
  pgm_sock_t*             _scktSend;
  struct pgm_addrinfo_t * _addrInfo;
  
  Receiver *              _receiver;

public:
  PGMSocket(NetworkOptions* options);
  ~PGMSocket();
  void listen();
  void setEventHandler(RawDataHandler * h);

  void send(const char *data, uint32_t size);
  void close();
};

typedef struct
{
  PGMSocket      * sckt;
  NetworkOptions * options;
}PGMSocketInfo;

typedef std::vector<PGMSocketInfo*>           PGMSocketInfoList;
typedef std::vector<PGMSocketInfo*>::iterator PGMSocketInfoListIterator;

class PGMSocketFactory
{
private:
  static PGMSocketFactory * _instance;
  PGMSocketInfoList _list;

private:
  PGMSocketFactory();

public:
  static PGMSocketFactory * GetInstance();
  ~PGMSocketFactory();
  
  PGMSocket * create(NetworkOptions *);
};

}}

#endif
