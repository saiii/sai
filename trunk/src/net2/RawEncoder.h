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
#include <net2/DataDescriptor.h>

namespace sai
{
namespace net2
{

class RawEncoder;
class XmlEncoder
{
friend class RawEncoder;
private:
  std::string& _msg;
  uint32_t     _opcode;
  std::string  _userData;

public:
  XmlEncoder(std::string& out);
  ~XmlEncoder();

  void setOpcode(uint32_t ocode) { _opcode = ocode; }
  void setUserData(std::string xml) { _userData = xml; }
  void pack(DataDescriptor&);
};

class BinaryEncoder
{
friend class RawEncoder;
private:
  std::string& _msg;

public:
  BinaryEncoder(std::string& out);
  ~BinaryEncoder();

  void add(char * data, uint32_t size);
  void pack(DataDescriptor&);
};

class RawEncoder
{
private:
  std::string& _msg;
  std::string  _back;

public:
  RawEncoder(XmlEncoder&, BinaryEncoder&, std::string& out);
  ~RawEncoder();

  void pack(DataDescriptor&);
};

}}

#endif
