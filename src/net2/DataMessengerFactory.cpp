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

#include <net2/Transport.h>
#include <net2/DataMessenger.h>
#include <net2/RawDecoder.h>
#include <net2/ProtId.h>
#include "DataMessengerFactory.h"

using namespace sai::net2;

DataMessengerFactory::DataMessengerFactory(NetworkOptions* opt, RawDataHandler * handler):
  _receiver(0),
  _decoder(0),
  _dispatcher(0)
{
  if (opt)
  {
    _receiver = new InternalTransport();
    if (handler)
    {
      _receiver->initialize(opt, handler);
    }
    else
    {
      _receiver->initialize(opt, this);
    }
  }

  _dispatcher = new DataDispatcher();
  _decoder = new RawDecoder(_dispatcher);
  _dispatcher->_old = _decoder->_old;
}

DataMessengerFactory::~DataMessengerFactory()
{
  while (_list.size() > 0)
  {
    DataMessenger * messenger = _list.front();
    _list.erase(_list.begin());
    delete messenger;
  }
  
  delete _dispatcher;
  delete _decoder;
  delete _receiver;
}

void 
DataMessengerFactory::processDataEvent(Endpoint * endpoint, const char * buffer, const uint32_t size)
{
  //if (endpoint->source() == TCP && PerfMeasure::InCount != ++PerfMeasure::OutCount)
  //{
  //  printf("Packet lost! (In %u) vs. (Out %u)\n", PerfMeasure::InCount, PerfMeasure::OutCount);
  //  PerfMeasure::OutCount = PerfMeasure::InCount;
  //}

  _decoder->processData(endpoint, buffer, size);
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

void 
DataMessengerFactory::destroy(DataMessenger * m)
{
  if (_list.size() > 0)
  {
    std::vector<DataMessenger*>::iterator iter;
    for (iter = _list.begin(); iter != _list.end(); iter++)
    {
      DataMessenger * messenger = *iter;
      if (messenger == m)
      {
        _list.erase(iter);
        delete messenger;
        return;
      }
    }
  }
}

