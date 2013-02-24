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

#ifndef __SAI_NET2_DATAMESSENGERFACTORY__
#define __SAI_NET2_DATAMESSENGERFACTORY__

#include <vector>
#include <net2/RawDecoder.h>
#include <net2/DataHandler.h>
#include <net2/DataDispatcher.h>
#include <net2/Transport.h>
#include <net2/DataMessenger.h>

namespace sai
{
namespace net2
{

class Endpoint;
class Transport;
class DataMessenger;
class InternalTransport;
class AuthenticationHandler;
class DataMessengerFactory : public RawDataHandler
{
friend class DataMessenger;
private:
  InternalTransport *          _receiver;
  RawDecoder *                 _decoder;
  DataDispatcher *             _dispatcher;
  std::vector<DataMessenger*>  _list;

public:
  DataMessengerFactory(NetworkOptions* opt, RawDataHandler * handler = 0);
  virtual ~DataMessengerFactory();
  void processDataEvent(Endpoint*, const char *, const uint32_t);

  DataMessenger *  create(Transport* transport);  
  void             destroy(DataMessenger * messenger);
  DataDispatcher * getDispatcher() { return _dispatcher; }
  PgmTransport *   getPgmTransport() { return _receiver->getPgmTransport(); }
};

}}

#endif
