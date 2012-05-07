//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#include <cstring>
#include "DataDescriptor.h"

using namespace sai::net2;

DataDescriptor::DataDescriptor():
  version(0)
{
  memset(&raw, 0, sizeof(raw));
  memset(&xml, 0, sizeof(xml));
  memset(&binary, 0, sizeof(binary));
  
}

DataDescriptor::~DataDescriptor()
{
  delete [] xml.x2.data;
  delete [] binary.b2.map;
  delete [] binary.b2.data;
}

