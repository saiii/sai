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

#ifndef __SAI_NET_DATACHAINABLE__
#define __SAI_NET_DATACHAINABLE__

#include <stdint.h>
#include <net/DataDispatchable.h>
#include <net/DataHandler.h>
#include <net/ChainFilter.h>

namespace sai 
{ 
namespace net 
{

class DataChainable : public DataHandler,
                      public DataDispatchable
{
protected:
  ChainFilter * _filter;

protected:
  virtual uint32_t decode(DataDescriptor&, std::string&) = 0;

public:
  DataChainable();
  virtual ~DataChainable();
  virtual void processDataEvent(DataDescriptor& desc, std::string& data)
  {  
    uint32_t id = decode(desc, data);
    if (id)
    {
      bool valid = true;
      if (valid && _filter) valid = _filter->filterEvent(desc, data);
      if (valid) dispatch(id, desc, data);
    }
  }
  void setFilter(ChainFilter * f) { _filter = f; }
  virtual void setChecker(DataChecker * c) { }
};

}
}
#endif
