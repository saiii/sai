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

#ifndef __SAI_NET2_DATADISPATCHER__
#define __SAI_NET2_DATADISPATCHER__

#include <map>
#include <net2/DataDescriptor.h>
#include <net2/DataHandler.h>
#include <net/DataHandler.h>

namespace sai
{
namespace net2
{

typedef std::map<uint32_t, DataHandler*>           DispatchTable;
typedef std::map<uint32_t, DataHandler*>::iterator DispatchTableIterator;

class DataDispatcher
{
friend class DataMessengerFactory;
friend class DataDispatcherTest;
private:
  DispatchTable   _table;
  DataHandler *   _defaultHandler;
  void *          _old;
  bool            _ignoreDestinationField;

protected:
  DataDispatcher();
  bool intlRegisterHandler(uint32_t id, DataHandler * handler);

public:
  virtual ~DataDispatcher();
  virtual void dispatch(DataDescriptor&, char * origMsg, uint32_t origSize);

  bool registerHandler(uint32_t id, DataHandler * handler);
  bool registerHandler(uint32_t id, sai::net::DataHandler * handler);
  void setDefaultHandler(DataHandler * handler);
  void setIgnoreDestinationField() { _ignoreDestinationField = true; }
};

}}

#endif
