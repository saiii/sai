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

#include "RawEncoder.h"

using namespace sai::net2;

XmlEncoder::XmlEncoder(std::string& out):
  _msg(out),
  _opcode(0)
{}

XmlEncoder::~XmlEncoder()
{}

void 
XmlEncoder::pack(DataDescriptor& desc)
{
}

BinaryEncoder::BinaryEncoder(std::string& out):
  _msg(out)
{
}

BinaryEncoder::~BinaryEncoder()
{
}

void 
BinaryEncoder::add(char * data, uint32_t size)
{
}

void 
BinaryEncoder::pack(DataDescriptor& desc)
{
}

RawEncoder::RawEncoder(XmlEncoder& xml, BinaryEncoder& bin, std::string& out):
  _msg(out)
{
  _back.append(xml._msg.data());
  _back.append(bin._msg.data());
}

RawEncoder::~RawEncoder()
{
}

void 
RawEncoder::pack(DataDescriptor& desc)
{
}

