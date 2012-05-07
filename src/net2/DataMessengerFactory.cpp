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

#include "Transport.h"
#include "DataMessenger.h"
#include "RawDecoder.h"
#include "DataMessengerFactory.h"

using namespace sai::net2;

DataMessengerFactory::DataMessengerFactory(std::string ip, uint16_t port, McastSet* mcastSet):
  _receiver(0),
  _decoder(0)
{
  _receiver = new InternalTransport();
  _receiver->initialize(ip, port, mcastSet, this);

  _decoder = new RawDecoder();
}

DataMessengerFactory::~DataMessengerFactory()
{
  while (_list.size() > 0)
  {
    DataMessenger * messenger = _list.front();
    _list.erase(_list.begin());
    delete messenger;
  }
  
  delete _decoder;
  delete _receiver;
}

void 
DataMessengerFactory::processDataEvent(const char * buffer, const uint32_t size)
{
  _decoder->processData(buffer, size);
}

DataMessenger * 
DataMessengerFactory::create(Transport* transport)
{
  if (_list.size() > 0)
  {
    for (uint32_t i = 0; i < _list.size(); i += 1)
    {
      DataMessenger * messenger = _list.at(i);
      if (messenger->_transport == transport)
      {
        return messenger;
      }
    }
  }

  DataMessenger * messenger = new DataMessenger(transport);
  _list.push_back(messenger);
  return messenger;
}

