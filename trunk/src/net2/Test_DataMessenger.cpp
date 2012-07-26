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
  static void generateData(DataDescriptor& desc);
};

class MyHandler : public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc);
};

void testUdp(int argc, char * argv[]);
void testUdpPoint2Point(int argc, char * argv[]);

int main(int argc, char * argv[])
{
#ifdef _WIN32
  char * a[] = {"xx", "client", "192.168.8.120", "192.168.8.120"};
  testUdp(4, a);
  //testUdpPoint2Point(4, a);
#else
  testUdp(argc, argv);
  //testUdpPoint2Point(argc, argv);
#endif
  return 0;
}

void testUdpPoint2Point_Client(int argc, char * argv[]);
void testUdpPoint2Point_Server(int argc, char * argv[]);
void 
testUdpPoint2Point(int argc, char * argv[])
{
  if (argc >= 2)
  {
    if (strcmp(argv[1], "client") == 0)
    {
      testUdpPoint2Point_Client(argc, argv);
    }
    else
    {
      testUdpPoint2Point_Server(argc, argv);
    }
  }
  else
  {
    fprintf(stderr, "Invalid parameters!\nUsage: %s <client|server>\n", argv[0]);
  }
}

void 
testUdpPoint2Point_Client(int argc, char * argv[])
{
  Net* net = Net::GetInstance();
  NicList * nicList = net->getNicList();
  nicList->detect();
  Nic * nic = nicList->getDefaultNic();

  std::string ip;
  nic->toString(ip);
  
  McastSet set;
  set.add("224.1.1.1");
  set.add("224.2.2.2");

  std::string addr = "0.0.0.0";
  if (argc == 4)
  {
    addr = argv[3];
  }
  DataMessengerFactory factory(addr, 8800, &set);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(100, &handler);


  UdpTransport* transport = new UdpTransport();
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
  transport->destination("192.168.8.148", 8808);

  DataMessenger * messenger = factory.create(transport);
  MyTimer t(messenger);

  net->mainLoop();
}

void 
testUdpPoint2Point_Server(int argc, char * argv[])
{
  Net* net = Net::GetInstance();
  NicList * nicList = net->getNicList();
  nicList->detect();
  Nic * nic = nicList->getDefaultNic();

  std::string ip;
  nic->toString(ip);
  
  McastSet set;
  set.add("224.3.3.3");
  set.add("224.4.4.4");

  std::string addr = "0.0.0.0";
  if (argc == 4)
  {
    addr = argv[3];
  }
  DataMessengerFactory factory(addr, 8808, &set);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(100, &handler);

  UdpTransport* transport = new UdpTransport();
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
  transport->destination("192.168.8.120", 8800);

  DataMessenger * messenger = factory.create(transport);
  MyTimer t(messenger);

  net->mainLoop();
}

void testUdp_Client(int argc, char * argv[]);
void testUdp_Server(int argc, char * argv[]);
void 
testUdp(int argc, char * argv[])
{
  if (argc >= 2)
  {
    if (strcmp(argv[1], "client") == 0)
    {
      testUdp_Client(argc, argv);
    }
    else
    {
      testUdp_Server(argc, argv);
    }
  }
  else
  {
    fprintf(stderr, "Invalid parameters!\nUsage: %s <client|server>\n", argv[0]);
  }
}

void 
testUdp_Client(int argc, char * argv[])
{
  Net* net = Net::GetInstance();
  NicList * nicList = net->getNicList();
  nicList->detect();
  Nic * nic = nicList->getDefaultNic();

  std::string ip;
  nic->toString(ip);
  
  McastSet set;
  set.add("224.1.1.1");
  set.add("224.2.2.2");

  std::string addr = "0.0.0.0";
  if (argc == 4)
  {
    addr = argv[3];
  }
  DataMessengerFactory factory(addr, 8800, &set);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(100, &handler);

  UdpTransport* transport = new UdpTransport();
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
  transport->destination("224.3.3.3", 8808);

  DataMessenger * messenger = factory.create(transport);
  MyTimer t(messenger);

  Net::GetInstance()->mainLoop();
  transport->close();
}

void 
testUdp_Server(int argc, char * argv[])
{
  Net* net = Net::GetInstance();
  NicList * nicList = net->getNicList();
  nicList->detect();
  Nic * nic = nicList->getDefaultNic();

  std::string ip;
  nic->toString(ip);
  
  McastSet set;
  set.add("224.3.3.3");
  set.add("224.4.4.4");

  std::string addr = "0.0.0.0";
  if (argc == 4)
  {
    addr = argv[3];
  }
  DataMessengerFactory factory(addr, 8808, &set);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(100, &handler);

  UdpTransport* transport = new UdpTransport();
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
  transport->destination("224.2.2.2", 8800);

  DataMessenger * messenger = factory.create(transport);
  //MyTimer t(messenger);

  net->mainLoop();
}

void 
MyHandler::processDataEvent(DataDescriptor& desc)
{
  desc.xmlDataReader->moveTo("mydata");
  std::string t = desc.xmlDataReader->get("time", "value");
  std::cout << "Time : " << t << std::endl;
}

MyTimer::MyTimer(DataMessenger * m):
  _msngr(m)
{
  schedule(3, 0);
}

MyTimer::~MyTimer()
{
}

void 
MyTimer::timerEvent()
{
  DataDescriptor desc;
  generateData(desc);
  _msngr->send(desc);
  std::cout << "Send data..." << std::endl;

  schedule();
}

void 
MyTimer::generateData(DataDescriptor& desc)
{
  time_t t = time(0);

  std::stringstream txt;
  txt << "<mydata>";
  txt   << "<time value=\"" << t <<"\" />";
  txt << "</mydata>";
  static std::string data;
  data.clear();
  data = txt.str();

  desc.raw.r2.encAlgo    = ENC_ALGO_NONE;
  desc.raw.r2.encTagSize = 0;
  desc.raw.r2.comAlgo    = COMP_ALGO_NONE;
  desc.raw.r2.comTagSize = 0;
  desc.raw.r2.hashAlgo   = HASH_ALGO_SHA256;
  desc.raw.r2.hashTagSize= 32;
  desc.raw.r2.xmlSize    = data.length();
  desc.raw.r2.binSize    = 0;
  desc.raw.r2.xmlData    = (char*)data.c_str();
  desc.raw.r2.binData    = 0;

  desc.protMessage->setId(100);
}

