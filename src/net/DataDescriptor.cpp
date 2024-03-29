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

#include <string>
#include <iostream>
#include <stdio.h>
#include "DataDescriptor.h"

using namespace sai::net;

void 
DataDescriptor::print()
{
  std::string str;
  char buf[1024];
  sprintf(buf, "%u\n", version);
  str.append("version = "); str.append(buf);

  std::string tmp;
  str.append("from = \n");
  from.toString(tmp);
  str.append(tmp);

  str.append("to = \n");
  to.toString(tmp);
  str.append(tmp); 

  std::cout << str << std::endl;
}

Address::Address() :
  ival(0)
{
}

void IP2String(uint32_t ip, char *output)
{
  char one[8],two[8],three[8],four[8];
  sprintf(one,   "%u",  ip >> 24);
  sprintf(two,   "%u", (ip >> 16) & 0xff);
  sprintf(three, "%u", (ip >> 8)  & 0xff);
  sprintf(four,  "%u",  ip & 0xff);
  sprintf(output, "%s.%s.%s.%s", one, two, three, four);
}

void 
Address::toString(std::string& ret, OutputType type)
{
  ret.clear();
  switch (type)
  {
    case LOG_MSG_SINGLE_LINE:
      {
        ret.append("Address { ");
        if (str.length() > 0)
        {
          ret.append(str);
        }
        else
        {
          char buf[64];
          IP2String(ival, buf);
          ret.append(buf);
        }
        ret.append(" }");
      }
      break;
    case LOG_MSG_MULTIPLE_LINE:
      {
        char buf[1024];
        ret.append("Address {\n");
        sprintf(buf, "0x%x\n", ival);
        ret.append("\tival = "); ret.append(buf);
        ret.append("\t str = "); ret.append(str); ret.append("\n");
        ret.append("}\n");
      }
      break;
    case RAW_MSG:
    default:
      {
        if (str.length() > 0)
        {
          ret.append(str);
        }
        else
        {
          char buf[64];
          IP2String(ival, buf);
          ret.append(buf);
        }
      }
      break;
  }
}
