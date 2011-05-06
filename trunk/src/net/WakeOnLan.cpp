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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <net/Socket.h>
#include <net/Net.h>

#include "WakeOnLan.h"

using namespace sai::net;

class WakeOnLanImpl
{
  public:

  WakeOnLanImpl(std::string address, std::string mac, uint32_t port)
  {
    ClientSocket * socket = ClientSocket::Create(
                              *Net::GetInstance(),
                              SAI_SOCKET_I_PROTOCOL,  SAI_SOCKET_OPT_UDP,
                              SAI_SOCKET_B_BROADCAST, SAI_SOCKET_OPT_TRUE,
                              SAI_SOCKET_EOA);
    socket->open();
    socket->connect(address, port);

    const uint32_t SIZE = 102;
    char * msg = new char[SIZE];
    char * ptr = msg;

    memset(ptr, 0xFF, 6); ptr += 6;

    char nMac [][3] = {
      {mac.at( 0), mac.at( 1), 0},
      {mac.at( 2), mac.at( 3), 0},
      {mac.at( 4), mac.at( 5), 0},
      {mac.at( 6), mac.at( 7), 0},
      {mac.at( 8), mac.at( 9), 0},
      {mac.at(10), mac.at(11), 0}};

    unsigned char uMac [6];
    for (uint16_t i = 0; i < 6; i += 1)
    {
      sscanf(nMac[i], "%x", &uMac[i]);
    }

    for (uint16_t i = 0; i < 16; i += 1)
    {
      memcpy(ptr, &uMac, sizeof(uMac));
      ptr += sizeof(uMac);
    }

    socket->send(msg, SIZE);

    socket->close();
    delete socket;
    delete [] msg;
  }
};

WakeOnLan::WakeOnLan()
{
}

void 
WakeOnLan::execute(std::string mac)
{
  WakeOnLanImpl(Net::GetInstance()->getLocalBroadcastAddress(), mac, 9);
}

WakeOnLan::~WakeOnLan()
{
}

