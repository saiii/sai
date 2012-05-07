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

#include <stdlib.h>
#include <utils/XmlReader.h>
#include <net/Exception.h>
#include "DataDispatcher.h"

using namespace sai::net2;

DataDispatcher::DataDispatcher():
  _defaultHandler(0)
{
}

DataDispatcher::~DataDispatcher()
{
  _table.clear();
}

void 
DataDispatcher::dispatch(DataDescriptor& desc)
{
  sai::utils::XmlReader reader;
  reader.parseMem(desc.xml.x2.data);
  reader.moveTo("protocol");
  std::string op = reader.get("opcode", "value");
 
  uint32_t opcode = atoi(op.c_str());

  desc.xmlReader = &reader;
  desc.xmlReader->moveTo("userdata");

  DispatchTableIterator iter;
  if ((iter = _table.find(opcode)) != _table.end())
  {
    DataHandler * handler = iter->second;
    handler->processDataEvent(desc);
    return;
  }

  if (_defaultHandler)
  {
    _defaultHandler->processDataEvent(desc);
  }
  else
  {
    throw sai::net::DataException("No suitable DataHandler for this opcode!");
  }
}

bool 
DataDispatcher::registerHandler(uint32_t opcode, DataHandler * handler)
{
  const bool NEW_ENTRY = true;
  const bool DUPLICATED = false;

  DispatchTableIterator iter = _table.find(opcode);
  if (iter != _table.end())
  {
    return DUPLICATED;
  }

  _table.insert(std::make_pair(opcode, handler));
  return NEW_ENTRY;
}

void 
DataDispatcher::setDefaultHandler(DataHandler * handler)
{
  _defaultHandler = handler;
}

