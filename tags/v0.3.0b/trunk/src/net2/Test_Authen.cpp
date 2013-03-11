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

#include <iostream>
#include <sstream>
#include <net2/Net.h>
#include <net2/Nic.h>
#include <net2/NicList.h>
#include <net2/ProtId.h>
#include <net2/RawEncoder.h>
#include <net2/DataHandler.h>
#include <net2/DataMessenger.h>
#include <net2/DataDescriptor.h>
#include <net2/DataMessengerFactory.h>
#include <net2/TimerTask.h>

using namespace sai::net2;

class MyTimer : public TimerTask
{
private:
  DataMessenger * _msngr;

public:
  MyTimer(DataMessenger * m);
  ~MyTimer();
  void timerEvent();
};

class MyHandler : public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc);
};

void testPgm(int argc, char * argv[]);

int main(int argc, char * argv[])
{
  Net* net = Net::GetInstance();
  NicList * nicList = net->getNicList();
  nicList->detect();
  Nic * nic = nicList->getDefaultNic();

  std::string addr = "0.0.0.0";
  if (argc == 4)
  {
    addr = argv[3];
  }

  std::string ip;
  nic->toString(ip);
  
  sai::net2::NetworkOptions options;
  options.setHttp(true);
  options.addReceive("224.1.1.1");
  options.setSend("224.2.2.2");

  options.setInterface(addr);
  options.setPort(8015);

  DataMessengerFactory factory(&options);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(10000, &handler);

  PgmTransport* transport = new PgmTransport();
  if (argc >= 3 && strcmp(argv[2], "default") == 0)
  {
    transport->bind(ip);
  }
  else if (argc >= 3)
  {
    transport->bind(argv[2]);
  }
  else
  {
    transport->bind("0.0.0.0");
  }
  //transport->destination("199.115.118.224", 8006);
  transport->destination("192.168.8.148", 8006);

  DataMessenger * messenger = factory.create(transport);
  MyTimer t(messenger);

  //SecuredLinkManager::GetInstance()->setFactory(&factory);
  //SecuredLinkManager::GetInstance()->setSecuredManagerLocation("199.115.118.224");
  //SecuredLinkManager::GetInstance()->setSecuredManagerLocation("192.168.8.148");
  //SecuredLinkManager::GetInstance()->setLoginData("John", "Doe");
  //SecuredLinkManager::GetInstance()->activate();

  Net::GetInstance()->mainLoop();
  transport->close();
  return 0;
}

void 
MyHandler::processDataEvent(DataDescriptor& desc)
{
  desc.raw->print();
}

MyTimer::MyTimer(DataMessenger * m):
  _msngr(m)
{
  schedule(1, 0);
}

MyTimer::~MyTimer()
{
}

void 
MyTimer::timerEvent()
{
  static int cnt = 0;
  if (++cnt == 2)
  {
    //SecuredLinkManager::GetInstance()->activate();
  }
  schedule();
}

