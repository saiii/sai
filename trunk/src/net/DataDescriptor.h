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
  typedef enum
  {
    LOG_MSG_SINGLE_LINE   = 0,
    LOG_MSG_MULTIPLE_LINE = 1,
    RAW_MSG               = 2
  }OutputType;

public:
  Address();
  void toString(std::string&, OutputType type = LOG_MSG_MULTIPLE_LINE);
  void toUInt(uint32_t&);
  Address& operator=(const Address& rhs) { ival = rhs.ival; str = rhs.str; return *this; }
};

class DataDescriptor
{
public:
  uint16_t version;
  char     sender[16];
  uint32_t seqNo;
  Address  from;
  Address  to;

  uint32_t opcode;

public:
  void print();

  DataDescriptor& operator=(const DataDescriptor& rhs);
};

}
}
#endif
