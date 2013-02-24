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
#include <sstream>
#include <net2/Net.h>
#include <net2/Transport.h>
#include <net2/DataHandler.h>

using namespace sai::net2;

class MyTcpTransportEventHandler : public sai::net2::TcpTransportEventHandler
{
public:
  void processConnectionClosedEvent() 
  { std::cout << "Closed" << std::endl; }

  void processDataEvent(sai::net2::Endpoint * endpoint, const char * buffer, const uint32_t bytes) 
  { 
    if (buffer == 0) return;
    std::cout << "::> " << bytes << std::endl;

    char buff[4096];

    char * ptr = 0;
    if ((ptr = (char*)strstr(buffer, "\r\n\r\n")) != 0)
    {
      if (bytes < 40) return;
      ptr += 4;
      char * ptr2 = (char*)strstr(ptr + 1, "\r\n");
      if (ptr2)
      {
        ptr2 += 2;
        memset(buff, 0, sizeof(buff));
        memcpy(buff, ptr2, bytes - (ptr2 - buffer));
        std::cout << buff << std::endl; 
      }
    }
    else
    {
      memset(buff, 0, sizeof(buff));
      memcpy(buff, buffer, bytes);
      std::cout << "Data: " << buff << std::endl; 
    }
  }
};

int main(int argc, char * argv[])
{
  MyTcpTransportEventHandler handler;
  TcpTransport* transport = new TcpTransport();
  transport->setHandler(&handler);
  // connection test
  //transport->connect("199.115.118.224", 2818);
  //transport->send("Hi There");

  transport->connect("199.115.114.39", 80);

  std::stringstream msg;
  std::stringstream dat;
  dat << "username=athipr%40gmail.com&password=0d791d1fed8b5ff8503f024de1b1ea0cc73136ccb80a6f8c69aa38eabef892ff";

  msg << "POST /authen/Login.php HTTP/1.1\r\n";
  msg << "Host: svc.manee.availablo.com\r\n";
  msg << "Accept: text/html\r\n";
  msg << "Content-Type: application/x-www-form-urlencoded\r\n";
  msg << "Content-Length: " << dat.str().size() << "\r\n\r\n";
  msg << dat.str().data();

  transport->send(msg.str().data(), msg.str().size());

  Net::GetInstance()->mainLoop();

  transport->close();
  delete transport;
  return 0;
}
