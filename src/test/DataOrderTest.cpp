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

class MsgChecker : public DataHandler
{
private:
  typedef struct
  {
    uint32_t cnt;
    uint32_t prv;
  }Cnt;
  typedef std::map<std::string,Cnt*>           CheckTable;
  typedef std::map<std::string,Cnt*>::iterator CheckTableIterator;

private:
  CheckTable _table;

public:
  MsgChecker()
  {
  }

  ~MsgChecker()
  {
    CheckTableIterator iter;
    while (_table.size() > 0)
    {
      iter = _table.begin();
      Cnt* i = iter->second;
      _table.erase(iter);
      delete i;
    }
  }

  void processDataEvent(DataDescriptor& desc, std::string& msg)
  {
    char sender[17];
    memcpy(sender, desc.sender, 16);
    sender[16] = 0;

    Cnt* n = 0;
    CheckTableIterator iter = _table.find(sender);
    if (iter == _table.end())
    {
      Cnt* num = new Cnt();
      num->cnt = 1;
      num->prv = 1;
      _table.insert(std::make_pair(sender, num));
      n = num;
      printf("Detected a new sender %s\n", sender);
    }
    else
    {
      n = iter->second;
    }
    
    bool prv = false;
    long long num = 0;
    if (msg.at(0) == 'P')
    {
      const char * ptr = msg.c_str();
      num = atoll(ptr+2);    
      prv = true;
    } 
    else
    {
      num = atoll(msg.c_str());
    }

    if (prv)
    {
      if (n->prv != (uint32_t)num)
      {
        abort();
      }
      n->prv += 1;
      //printf("(%s:PRV:%09u) OK\n", sender, num);
    }
    else
    {
      if (n->cnt != (uint32_t)num)
      {
        abort();
      }
      n->cnt += 1;

      if ((n->cnt % 50) == 0)
      {
        //printf("(%s:   :%09u) OK\n", sender, num);
      }
    }
  }
};

class MsgSender : public TimerTask 
{
private:
  uint32_t _at;
  uint32_t _repeat;
  uint32_t _cnt;
  uint32_t _prvCnt;
  uint32_t _repCnt;
  bool     _repMode;

public:
  MsgSender(uint32_t at, uint32_t repeat):
    _at(at), _repeat(repeat), _cnt(0), _prvCnt(0), _repCnt(0), _repMode(false)
  {
  }

  ~MsgSender()
  {
  }

  void timerEvent()
  {
    char buf[128];
    sprintf(buf, "%u", ++_cnt);

    if ((_cnt % _at) == 0 || _repMode)
    {
      _repMode = true;

      char buff[128];
      sprintf(buff, "%u", ++_prvCnt);
      std::string pData = "P:";
      pData.append(buff);
      DataBus::GetInstance()->sendPointToPoint("Yo", 1, pData);
      //printf("(PRV:%09u)\n", _prvCnt);

      if (++_repCnt >= _repeat)
      {
        _repMode = false;
        _repCnt  = 0;
      }
    }

    DataBus::GetInstance()->send("Yo", 1, buf);
    //printf("(   :%09u)\n", _cnt);
    schedule();
  }
};

int main(int argc, char * argv[])
{
  bool checker = true;
  uint32_t at = 0;
  uint32_t repeat = 0;
  switch (argc)
  {
    case 2:
      if (strcmp(argv[1],"-checker") == 0)
      {
        checker = true;
      }
      else if (strcmp(argv[1], "-sender") == 0)
      {
        checker = false;
      }
      else
      {
        printf("Invalid argument!!!\n");
        return 1;
      }
      break;
    case 3:
      {
        char * freq = strdup(argv[2]);
        char * colon = 0;
        if (strcmp(argv[1],"-sender") == 0 && ((colon = strchr(freq, ':')) != 0))
        {
          checker = false;
          char * ptrAt  = freq;
          char * ptrRep = colon + 1; 
          *colon = 0;
          at     = atoi(ptrAt);
          repeat = atoi(ptrRep);
        }
        else
        {
          printf("Invalid argument!!!\n");
          return 1;
        }
        free(freq);
      }
      break;
    default:
      printf("Invalid argument!!!\n");
      printf("Usage: %s <-checker|-sender <at:repeat>>\n", argv[0]);
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
  channel.setLocalAddress("0.0.0.0");
  if (checker)
  {
    channel.setSendMcast("224.1.1.1");
    channel.addRecvMcast("224.2.2.2");
  }
  else
  {
    channel.setSendMcast("224.2.2.2");
    channel.addRecvMcast("224.1.1.1");
  }

  DataBus *bus = DataBus::GetInstance();
  bus->setChannel(&channel);

  MsgChecker chk;
  if (checker)
  {
    bus->registerHandler(1, &chk); 
  }

  bus->listen("Yo");
  bus->activate();

  MsgSender snd(at, repeat);
  if (!checker)
  {
    snd.schedule(0, 850);
  }

  Net::GetInstance()->mainLoop();

  return 0;
}
