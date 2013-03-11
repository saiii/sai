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

#include <stdint.h>
#include <iostream>
#include <string>

#define VERSION_MAJOR_STR "1"
#define VERSION_MINOR_STR "0"
#define VERSION_COUNT_STR "0"
#define VERSION_SUFFI_STR "a"

static std::string _version  = "(" VERSION_MAJOR_STR "." VERSION_MINOR_STR "." VERSION_COUNT_STR VERSION_SUFFI_STR ")";

std::string GetNet2Version()
{
  return _version;
}

const char * cGetNet2Version()
{
  return _version.c_str();
}

void PrintNet2Version()
{
  std::cout << GetNet2Version() << std::endl;
}

