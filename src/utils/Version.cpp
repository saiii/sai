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
#define VERSION_MINOR_STR "2"
#define VERSION_COUNT_STR "2"
#define VERSION_SUFFI_STR "a"

#ifdef _WIN32
#include <stdint.h>
#include <windows.h>
void MyMessageBox(const char * msg)
{
  uint32_t size = strlen(msg);

#ifdef _UNICODE
  wchar_t tmp[1024];
  mbstowcs(tmp, msg, size);
  ::MessageBox(0, tmp, L"", MB_OK);
#else
  ::MessageBox(0, msg, "", MB_OK);
#endif
}
#endif

static std::string _version  = "(libSai) Version " VERSION_MAJOR_STR "." VERSION_MINOR_STR "." VERSION_COUNT_STR VERSION_SUFFI_STR;
static std::string _version2 = "(" VERSION_MAJOR_STR "." VERSION_MINOR_STR "." VERSION_COUNT_STR VERSION_SUFFI_STR ")";

std::string SaiGetVersion()
{
  return _version;
}

std::string SaiGetNet2Version()
{
  return _version2;
}

void SaiPrintVersion()
{
  std::cout << SaiGetVersion() << std::endl;
}

