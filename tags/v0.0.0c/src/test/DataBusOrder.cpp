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

#include <stdio.h>
#include <string.h>
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
    "Unix (officially trademarked as UNIX, sometimes also ",
    "written as Unix) is a multitasking, multi-user",
    " computer operating system originally developed in 1969 by",
    " a group of AT&T employees at Bell Labs, including Ken Thompson,",
    " Dennis Ritchie, Brian Kernighan, Douglas McIlroy, ",
    "and Joe Ossanna. The Unix operating system was first ",
    "developed in assembly language, but by 1973 had been ",
    "almost entirely recoded in C, greatly facilitating its",
    " further development and porting to other hardware. Today",
    "'s Unix systems are split into various branches, developed",
    " over time by AT&T as well as various commercial vendors a",
    "nd non-profit organizations.",
    "The Open Group, an industry standards consortium, owns the .",
    "UNIX. trademark. Only systems fully compliant with and cer",
    "tified according to the Single UNIX Specification are quali",
    "fied to use the trademark; others might be called \"Unix sy",
    "stem-like\" or \"Unix-like\" (though the Open Group disappr",
    "oves of this term).[1] However, the term \"Unix\" is often ",
    "used informally to denote any operating system that closely",
    " resembles the trademarked system."};

class Sender : public TimerTask
{
public:
  int count;


  Sender()
  {
    count = 0;
  }

  void timerEvent()
  {
    if (count < 20)
    {
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
  sender->schedule(5, 0); 

  Net::GetInstance()->mainLoop();

  return 0;
}

