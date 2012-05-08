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

#include <cstring>
#include <sstream>
#include "RawEncoder.h"

#define NET_BUFFER_SIZE 8192

using namespace sai::net2;

BinaryEntry::BinaryEntry():
  data(0),
  offset(0),
  size(0)
{
}

BinaryEntry::~BinaryEntry()
{
  delete [] data;
}

RawEncoder::RawEncoder():
  _buffer(0),
  _opcode(0)
{
  _buffer = new char[NET_BUFFER_SIZE];
}

RawEncoder::~RawEncoder()
{
  while (_binList.size() > 0)
  {
    BinaryEntry * entry = _binList.front();
    _binList.erase(_binList.begin());
    delete entry;
  }
  delete [] _buffer;
}

void 
RawEncoder::setOpcode(uint32_t opcode)
{
  _opcode = opcode;
}

// name can only be alphanumeric
void 
RawEncoder::appendFieldValue(std::string name, std::string value)
{
  std::stringstream txt;
  std::string val = sai::utils::XmlReader::EncodeSpecialCharacter(value);
  txt << "<" << name << " value=\"" << val << "\" />";
  _data.append(txt.str());
}

void 
RawEncoder::appendXml(std::string xml)
{
  _data.append(xml);
}

void 
RawEncoder::appendBinary(std::string binaryData)
{
  BinaryEntry * entry = new BinaryEntry();
  entry->data = new char[binaryData.size()];
  memcpy(entry->data, binaryData.data(), binaryData.size());
  entry->offset = 0;
  entry->size   = binaryData.size();
  _binList.push_back(entry);
}

char*
RawEncoder::pack(DataDescriptor& desc, uint32_t& size)
{
  // TODO Put things to buffer, encrypt + compress here
  return 0;
}

