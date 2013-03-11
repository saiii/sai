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
#include <stdlib.h>
#include <string>
#include <map>

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

class Dummy : public TimerTask
{
public:
  void timerEvent()
  {
    printf("Hello\n");
    schedule();
  }
};

class MsgWriter : public DataHandler
{
private:
  typedef std::map<std::string,FILE*>           FileTable;
  typedef std::map<std::string,FILE*>::iterator FileTableIterator;
  FileTable _table;

public:
  MsgWriter()
  {
  }
 
  ~MsgWriter()
  {
    FileTableIterator iter;
    while (_table.size() > 0)
    {
      iter = _table.begin();
      FILE * fp = iter->second;
      _table.erase(iter);
      fclose(fp);
    }
  }

  void processDataEvent(DataDescriptor& desc, std::string& msg)
  {
    std::string name;
    desc.from.toString(name, Address::RAW_MSG);
    name.append("-");
    char buf [17];
    memcpy(buf, desc.sender, 16);
    buf[16] = 0;
    name.append(buf);
    name.append(".txt");

    FileTableIterator iter;
    iter = _table.find(name);
    if (iter == _table.end())
    {
      FILE * fp = fopen(name.c_str(), "w+");
      _table.insert(std::make_pair(name, fp));
    }

    iter = _table.find(name);
    FILE * fp = iter->second;

    fprintf(fp, "%s\n", msg.c_str());
    fflush(fp);
  }
};

int main(int argc, char * argv[])
{
  bool clientMode = true;
  bool writeMode = false;
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
        printf("Invalid argument!!!\n");
        return 1;
      }
      break;
    case 3:
      if (strcmp(argv[1],"-client") == 0)
      {
        clientMode = true;
        msg = argv[2];
      }
      else if (strcmp(argv[1],"-server") == 0 && strcmp(argv[2], "-write") == 0)
      {
        clientMode = false;
        writeMode = true;
      }
      else
      {
        printf("Invalid argument!!!\n");
        return 1;
      }
      break;
    default:
      printf("Invalid argument!!!\n");
      printf("Usage: %s <-server|-client <data>>\n", argv[0]);
	  return 0;
  }

  McastDataBusChannel channel;
#ifndef _WIN32
  if (getenv("SAI_TEST_PORT"))
  {
    channel.setPort(atoi(getenv("SAI_TEST_PORT")));
  }
  else
  {
    channel.setPort(1500);
  }
#else
  channel.setPort(1500);
#endif
  channel.setLocalAddress("eth0");
  if (clientMode)
  {
    channel.setSendMcast("224.1.1.1");
    channel.addRecvMcast("224.2.2.2");
    prompt = "client";
  }
  else
  {
    channel.setSendMcast("224.2.2.2");
    channel.addRecvMcast("224.1.1.1");
    prompt = "server";
  }

  DataBus *bus = DataBus::GetInstance();
  bus->setChannel(&channel);

  MsgPrinter printer;
  MsgWriter writer;
  if (writeMode)
  {
    bus->registerHandler(1888, &writer); 
  }
  else
  {
    bus->registerHandler(1888, &printer); 
  }

  bus->listen("Yo");
  bus->activate();

  
  if (clientMode)
  {
    bus->send("Yo", 1888, msg);
  }
  else
  {
    Dummy * dummy = new Dummy();
    dummy->schedule(3600, 0);
  }
  Net::GetInstance()->mainLoop();

  return 0;
}
