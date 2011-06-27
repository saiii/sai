//=============================================================================
// Copyright (C) 2011 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <boost/asio.hpp>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include <net/Net.h>
#include <net/DataBus.h>
#include <net/DataHandler.h>
#include <net/TimerTask.h>

using namespace sai::net;

std::string prompt;

class MsgPrinter : public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc, std::string& msg)
  {
    printf("(%s): %s\n", prompt.c_str(), msg.c_str());
  }
};

  std::string msg [] = { // 20 lines
    "[ 1]Unix (officially trademarked as UNIX, sometimes also ",
    "[ 2]written as Unix) is a multitasking, multi-user",
    "[ 3] computer operating system originally developed in 1969 by",
    "[ 4] a group of AT&T employees at Bell Labs, including Ken Thompson,",
    "[ 5] Dennis Ritchie, Brian Kernighan, Douglas McIlroy, ",
    "[ 6]and Joe Ossanna. The Unix operating system was first ",
    "[ 7]developed in assembly language, but by 1973 had been ",
    "[ 8]almost entirely recoded in C, greatly facilitating its",
    "[ 9] further development and porting to other hardware. Today",
    "[10]'s Unix systems are split into various branches, developed",
    "[11] over time by AT&T as well as various commercial vendors a",
    "[12]nd non-profit organizations.",
    "[13]The Open Group, an industry standards consortium, owns the .",
    "[14]UNIX. trademark. Only systems fully compliant with and cer",
    "[15]tified according to the Single UNIX Specification are quali",
    "[16]fied to use the trademark; others might be called \"Unix sy",
    "[17]stem-like\" or \"Unix-like\" (though the Open Group disappr",
    "[18]oves of this term).[1] However, the term \"Unix\" is often ",
    "[19]used informally to denote any operating system that closely",
    "[20] resembles the trademarked system."};

class Sender : public TimerTask
{
public:
  int         count;
  std::string customAddress;

  Sender()
  {
    count = 0;
  }

  void timerEvent()
  {
    if (count < 20)
    {
      if (count == 8 && customAddress.length() > 0)
      {
        DataBus::GetInstance()->sendPointToPoint(customAddress, 1, "<PRIVATE MESSAGE>");
      }
      else if (count == 15 && customAddress.length() > 0)
      {
        DataBus::GetInstance()->sendPointToPoint(customAddress, 1, "<PRIVATE MESSAGE 2>");
      }
      DataBus::GetInstance()->send("Yo", 1, msg[count]);
      count++;
    }
    else
    {  
      printf("done...\n");
    }
    schedule();
  }
};

int main(int argc, char * argv[])
{
  boost::asio::io_service svc;

  McastDataBusChannel channel;
  if (getenv("SAI_TEST_PORT"))
  {
    channel.setPort(atoi(getenv("SAI_TEST_PORT")));
  }
  else
  {
    channel.setPort(1500);
  }
  channel.setLocalAddress("0.0.0.0");
  channel.setSendMcast("224.1.1.1");
  channel.addRecvMcast("224.2.2.2");

  DataBus *bus = DataBus::GetInstance();
  bus->setChannel(&channel);

  MsgPrinter printer;
  bus->registerHandler(1, &printer); 

  bus->listen("Yo");
  bus->activate();

  Sender* sender = new Sender();

  if (argc > 1)
  {
    sender->customAddress = argv[1];
  }

  sender->schedule(2, 0); 

  Net::GetInstance()->mainLoop();

  return 0;
}

