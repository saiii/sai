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

#ifndef __SAI_NET_PROTOCOLENCODER__
#define __SAI_NET_PROTOCOLENCODER__

#include <stdint.h>
#include <net/DataDescriptor.h>

namespace sai 
{ 
namespace net 
{

class ProtocolEncoder
{
public:
  ProtocolEncoder();
  ~ProtocolEncoder();

  void encode(DataDescriptor&, uint32_t, std::string&, std::string&);
};

}
}
#endif
