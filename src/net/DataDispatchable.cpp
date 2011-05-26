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

#include "DataDispatchable.h"
#include "DataOrderingManager.h"
#include "DataHandler.h"
#include "Net.h"
#include "Exception.h"

using namespace sai::net;

DataDispatchable::DataDispatchable():
  _defaultHandler(0),
  _useChecker(false),
  _replay(false)
{
}

void 
DataDispatchable::dispatch(uint32_t opcode, DataDescriptor& desc, std::string data)
{
  if (_useChecker && !_replay)
  {
    if (DataOrderingManager::REL != DataOrderingManager::GetInstance()->receive(desc, data))
    {
      return;
    }
  }

  DispatchTableIterator iter;
  if ((iter = _table.find(opcode)) != _table.end())
  {
    DataHandler * handler = iter->second;
    handler->processDataEvent(desc, data);
    return;
  }

  if (_defaultHandler)
  {
    _defaultHandler->processDataEvent(desc, data);
  }
  else
  {
    throw DataException("No suitable DataHandler for this opcode!");
  }
}

DataDispatchable::~DataDispatchable()
{
  _table.clear();
}

bool 
DataDispatchable::registerHandler(uint32_t opcode, DataHandler * handler)
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
DataDispatchable::setDefaultHandler(DataHandler * handler)
{
  _defaultHandler = handler;
}

