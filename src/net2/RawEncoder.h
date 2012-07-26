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

#ifndef __SAI_NET2_RAWENCODER__
#define __SAI_NET2_RAWENCODER__

#include <stdint.h>
#include <string>
#include <vector>
#include <net2/DataDescriptor.h>

namespace sai
{
namespace net2
{

class RawEncoder
{
private:
  char *   _buffer;
  uint32_t _size;

public:
  RawEncoder();
  ~RawEncoder();

  char *   make(DataDescriptor& desc, uint32_t& size);
  char *   get() { return _buffer; }
  uint32_t size(){ return _size; }
};

}}

#endif
