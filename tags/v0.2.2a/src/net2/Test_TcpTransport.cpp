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
#include <net2/DataHandler.h>

using namespace sai::net2;

class MyTcpTransportEventHandler : public sai::net2::TcpTransportEventHandler
{
public:
  void processConnectionClosedEvent() 
  { std::cout << "Closed" << std::endl; }

  void processDataEvent(const char * buffer, const uint32_t bytes) 
  { 
    if (buffer == 0) return;

    char buff[4096];
    memset(buff, 0, sizeof(buff));
    memcpy(buff, buffer, bytes);
    std::cout << "Data: " << buff << std::endl; 
  }
};

int main(int argc, char * argv[])
{
  MyTcpTransportEventHandler handler;
  TcpTransport* transport = new TcpTransport();
  transport->setHandler(&handler);
  // connection test
  transport->connect("50.23.58.155", 2818);
  transport->send("Hi There");

  Net::GetInstance()->mainLoop();

  transport->close();
  delete transport;
  return 0;
}
