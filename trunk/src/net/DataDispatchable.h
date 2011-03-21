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

#ifndef __SAI_NET_DATADISPATCHABLE__
#define __SAI_NET_DATADISPATCHABLE__

#include <stdint.h>
#include <string>
#include <map>

namespace sai 
{ 
namespace net 
{

class DataHandler;
class DataDescriptor;
typedef std::map<uint32_t, DataHandler*>           DispatchTable;
typedef std::map<uint32_t, DataHandler*>::iterator DispatchTableIterator;

class DataOrderingManager;
class DataDispatchable
{
friend class SenderProfile;
private:
  DispatchTable        _table;
  DataHandler         *_defaultHandler;
  bool                 _useChecker;

protected:
  DataDispatchable();
  void dispatch(uint32_t, DataDescriptor&, std::string);

public:
  virtual ~DataDispatchable();
  bool registerHandler(uint32_t id, DataHandler * handler);
  void setDefaultHandler(DataHandler * handler);
 
  void activateChecker() { _useChecker = true; }
};

}
}
#endif
