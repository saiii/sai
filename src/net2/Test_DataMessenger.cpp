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
  std::string     _to;
  std::string     _type;

public:
  MyTimer(DataMessenger * m, std::string to, std::string type);
  ~MyTimer();
  void timerEvent();
  static void generateData(DataDescriptor& desc, std::string type);
};

class MyHandler : public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc);
};

void testPgm(int argc, char * argv[]);
void testPgmPoint2Point(int argc, char * argv[]);

int main(int argc, char * argv[])
{
#ifdef _WIN32
  char * a[] = {"xx", "client", "192.168.8.141", "192.168.8.141"};
  testPgm(4, a);
  //testPgmPoint2Point(4, a);
#else
  testPgm(argc, argv);
  //testPgmPoint2Point(argc, argv);
#endif
  return 0;
}

void testPgm_Client(int argc, char * argv[]);
void testPgm_Server(int argc, char * argv[]);
void 
testPgm(int argc, char * argv[])
{
  if (argc >= 2)
  {
    if (strcmp(argv[1], "client") == 0)
    {
      testPgm_Client(argc, argv);
    }
    else
    {
      testPgm_Server(argc, argv);
    }
  }
  else
  {
    fprintf(stderr, "Invalid parameters!\nUsage: %s <client|server>\n", argv[0]);
  }
}

void 
testPgm_Client(int argc, char * argv[])
{
  //Net* net = Net::GetInstance();
  //NicList * nicList = net->getNicList();
  //nicList->detect();
  //Nic * nic = nicList->getDefaultNic();

  std::string ip = argv[2];
  //nic->toString(ip);
  
  NetworkOptions options;
  options.setHttp(true);
  options.addReceive(argv[3]);
  options.setSend(argv[4]);

  options.setInterface(ip);
  options.setPort(atoi(argv[5]));

  DataMessengerFactory factory(&options);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(10000, &handler);

  PgmTransport* transport = new PgmTransport();
  transport->setOptions(&options);

  DataMessenger * messenger = factory.create(transport);
  std::string to = ".*";
  if (argc > 6)
    to = argv[6];
  MyTimer t(messenger, to, "client");

  Net::GetInstance()->mainLoop();
  transport->close();
}

void 
testPgm_Server(int argc, char * argv[])
{
  Net* net = Net::GetInstance();

  std::string ip = argv[2];
  
  NetworkOptions options;
  options.setHttp(true);
  options.addReceive(argv[3]);
  options.setSend(argv[4]);

  options.setInterface(ip);
  options.setPort(atoi(argv[5]));

  DataMessengerFactory factory(&options);

  MyHandler handler;
  factory.getDispatcher()->registerHandler(10000, &handler);

  PgmTransport* transport = new PgmTransport();
  transport->setOptions(&options);

  DataMessenger * messenger = factory.create(transport);
  std::string to = ".*";
  if (argc > 6)
    to = argv[6];
  MyTimer t(messenger, to, "server");

  net->mainLoop();
}

void 
MyHandler::processDataEvent(DataDescriptor& desc)
{
  sai::utils::XmlReader reader;
  reader.parseMem(desc.raw->xmlData);
  reader.moveTo("mydata");
  std::string t, type;
  reader.get("time", "value", t);
  reader.get("type", "value", type);
  std::cout << type << " Time : " << t << std::endl;
}

MyTimer::MyTimer(DataMessenger * m, std::string to, std::string type):
  _msngr(m),
  _to(to),
  _type(type)
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
  generateData(desc, _type);
  desc.protMessage->setDestination(_to);
  _msngr->send(desc);
  std::cout << "Send data..." << std::endl;

  schedule();
}

void 
MyTimer::generateData(DataDescriptor& desc, std::string type)
{
  time_t t = time(0);

  std::stringstream txt;
  txt << "<mydata>";
  txt   << "<time value=\"" << t <<"\" />";
  txt   << "<type value=\"" << type <<"\" />";
  txt << "</mydata>";
  static std::string data;
  data.clear();
  data = txt.str();

  desc.raw->encAlgo    = ENC_ALGO_NONE;
  desc.raw->encTagSize = 0;
  desc.raw->comAlgo    = COMP_ALGO_NONE;
  desc.raw->comTagSize = 0;
  desc.raw->hashAlgo   = HASH_ALGO_SHA256;
  desc.raw->hashTagSize= 32;
  desc.raw->xmlSize    = data.length();
  desc.raw->binSize    = 0;
  desc.raw->xmlData    = (char*)data.c_str();
  desc.raw->binData    = 0;

  desc.protMessage->setId(10000);
}

