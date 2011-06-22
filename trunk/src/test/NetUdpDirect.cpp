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

// SAI [ 23 Jun 2011 ]
#include <stdio.h>
#include <iostream>
#include <boost/asio.hpp>
#include <net/Socket.h>
#include <net/Net.h>
#include <net/TimerTask.h>

using namespace sai::net;

class Printer : public SocketEventHandler
{
public:
  void processDataEvent(char *data, uint32_t size)
  {
    printf("%s\n", data);
  }
};

int receiver(int argc, char *argv[])
{
  boost::asio::io_service io_service;
  Printer printer;
  ServerSocket * socket = ServerSocket::Create(*Net::GetInstance(),
                            SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_UDP,
                            SAI_SOCKET_B_REUSE_ADDR, SAI_SOCKET_OPT_TRUE,
                            SAI_SOCKET_EOA);
  if (argc < 3)
  {
    socket->bind("0.0.0.0", 1500);
  }
  else
  {
    socket->bind(argv[2], 1500);
  }
  socket->setEventHandler(&printer);
  socket->open();
  socket->listen();
  Net::GetInstance()->mainLoop();
  delete socket;
  return 0;
}

class Sender : public TimerTask
{
private:
  uint32_t _cnt;
  ClientSocket * _scket;

public:
  Sender(ClientSocket* scket): _cnt(0), _scket(scket)
  {
  }

  ~Sender()
  {
  }

  void timerEvent()
  {
    char msg[32];
    sprintf(msg, "%u", ++_cnt);
    _scket->send(msg, strlen(msg) + 1);
    if (_cnt >= 10000) return; 
    else schedule(0, 1);
  }
};

int sender(int argc, char * argv[])
{
  boost::asio::io_service io_service;
  ClientSocket * scket = ClientSocket::Create(
                            *Net::GetInstance(), 
                            SAI_SOCKET_I_PROTOCOL, SAI_SOCKET_OPT_UDP,
                            SAI_SOCKET_EOA);
  scket->open();
  scket->connect(argv[2], 1500);

  Sender s(scket);
  s.schedule(1, 0);

  Net::GetInstance()->mainLoop();
  delete scket;
  return 0;
}

int main(int argc, char * argv[])
{
  if (argc >= 2) 
  {
    if (strcmp(argv[1], "sender") == 0 && argc >= 3)
    {
      return sender(argc, argv);
    }
    else if (strcmp(argv[1], "receiver") == 0)
    {
      return receiver(argc, argv);
    }
  }

  std::cerr << "Invalid arguments!" << std::endl << "Usage: " << argv[0] << " <sender <ip> | receiver [ip address for binding interface card]>" << std::endl;
  return 1;
}
