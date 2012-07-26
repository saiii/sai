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

#include <net2/RawEncoder.h>
#include <net2/Transport.h>
#include "DataMessenger.h"

using namespace sai::net2;

DataMessenger::DataMessenger(Transport* transport):
  _transport(transport)
{
}

DataMessenger::~DataMessenger()
{
}

bool 
DataMessenger::send(DataDescriptor& desc)
{
  uint32_t size = 0;
  RawEncoder encoder;
  char * ptr = encoder.make(desc, size);
  _transport->send(ptr, size);
  return true;
}

DataMessengerGroup::DataMessengerGroup()
{
}

DataMessengerGroup::~DataMessengerGroup()
{
}

void 
DataMessengerGroup::add(DataMessenger * messenger)
{
  _list.push_back(messenger);
}

void 
DataMessengerGroup::remove(DataMessenger * messenger)
{
  if (_list.size() <= 0)
  {
    return;
  }

  DataMessengerListIterator iter;
  for(iter = _list.begin(); iter != _list.end(); iter ++)
  {
    DataMessenger * msg = *iter;
    if (msg == messenger)
    { 
      _list.erase(iter);
      return;
    }
  }
}

