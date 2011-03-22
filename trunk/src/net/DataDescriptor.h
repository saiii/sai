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

#ifndef __SAI_NET_DATADESCRIPTOR__
#define __SAI_NET_DATADESCRIPTOR__

#include <stdint.h>
#include <string>

namespace sai 
{ 
namespace net 
{

class Address
{
public:
  uint32_t    ival;
  std::string str;

public:
  Address();
  void toString(std::string&, bool singleLineOutput = false);
};

class DataDescriptor
{
public:
  uint16_t version;
  char     sender[16];
  uint32_t id;
  Address  from;
  Address  to;

  uint32_t opcode;
public:
  void print();
};

}
}
#endif
