//=============================================================================
// Copyright (C) 2011 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <iostream>
#include <string>

#define VERSION_MAJOR_STR "0"
#define VERSION_MINOR_STR "0"
#define VERSION_COUNT_STR "1"
#define VERSION_SUFFI_STR "b"

static std::string _version  = "(libSai) Version " VERSION_MAJOR_STR "." VERSION_MINOR_STR "." VERSION_COUNT_STR VERSION_SUFFI_STR;

std::string GetVersion()
{
  return _version;
}

const char * cGetVersion()
{
  return _version.c_str();
}

void PrintVersion()
{
  std::cout << GetVersion() << std::endl;
}

