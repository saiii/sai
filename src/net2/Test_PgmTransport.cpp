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

#include <cstring>
#include <iostream>
#include <net2/Net.h>
#include <net2/Transport.h>
#include <net2/PGMSocket.h>
#include <net2/DataHandler.h>

using namespace sai::net2;

int main(int argc, char * argv[])
{
  if (argc != 4) 
  {
    fprintf(stderr, "Usage: %s <local addr> <mcast> <port>\n", argv[0]);
    return 1;
  }

  NetworkOptions options;
  options.setHttp(true);
  options.addReceive(argv[2]);
  options.setSend("224.1.1.1");

  options.setInterface(argv[1]);
  options.setPort(atoi(argv[3]));

  PGMSocket sockt(&options);

  PgmTransport* transport = new PgmTransport();
  transport->setOptions(&options);

  std::string msg = "Hi!!!!";
  transport->send(msg.data(), msg.size()+1);

  Net::GetInstance()->mainLoop();

  transport->close();
  delete transport;
  return 0;
}