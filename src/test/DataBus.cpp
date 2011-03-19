//=============================================================================
// Copyright (C) 2009 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <stdio.h>
#include <string.h>
#include <string>

#include <net/Net.h>
#include <net/DataBus.h>
#include <net/DataHandler.h>

using namespace sai::net;

class ServerMsgPrinter : public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc, std::string& msg)
  {
    printf("(server): %s\n", msg.c_str());
  }
};

int main(int argc, char * argv[])
{
  bool clientMode = true;
  std::string msg = "Hello, server";
  switch (argc)
  {
    case 2:
      if (strcmp(argv[1],"-client") == 0)
      {
        clientMode = true;
      }
      else if (strcmp(argv[1], "-server") == 0)
      {
        clientMode = false;
        msg = "Hi! client";
      }
      else
      {
        fprintf(stderr, "Invalid argument!!!\n");
        return 1;
      }
      break;
    case 3:
      if (strcmp(argv[1],"-client") == 0)
      {
        clientMode = true;
        msg = argv[2];
      }
      else
      {
        fprintf(stderr, "Invalid argument!!!\n");
        return 1;
      }
      break;
    default:
      fprintf(stderr, "Invalid argument!!!\n");
      fprintf(stderr, "Usage: %s <-server|-client <data>>\n", argv[0]);
      return 0;
  }

  boost::asio::io_service svc;
  Net net(svc);

  McastDataBusChannel channel;
  channel.setPort(1500);
  channel.setLocalAddress("0.0.0.0");
  if (clientMode)
  {
    channel.setSendMcast("224.1.1.1");
    channel.addRecvMcast("224.2.2.2");
  }
  else
  {
    channel.setSendMcast("224.2.2.2");
    channel.addRecvMcast("224.1.1.1");
  }

  DataBus bus(net, &channel);

  ServerMsgPrinter printer;
  bus.registerHandler(1, &printer); 

  bus.listen("Yo");
  bus.activate();

  
  if (clientMode)
  {
    bus.send("Yo", 1, msg);
  }
  else
  {
    svc.run();
  }

  return 0;
}
