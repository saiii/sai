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

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <cstring>
#include <cstdio>
#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include "DataDescriptor.h"

using namespace sai::net2;

DataDescriptor::DataDescriptor():
  version(2),
  xmlProtReader(0),
  xmlDataReader(0),
  protMessage(0)
{
  memset(&raw, 0, sizeof(raw));
  protMessage = new ProtMessage();
}

DataDescriptor::~DataDescriptor()
{
  delete protMessage;
}

extern std::string SaiGetNet2Version();
boost::uuids::uuid ProtMessage::_uuid = boost::uuids::random_generator()();

ProtMessage::ProtMessage()
{
  const std::string uuid = boost::uuids::to_string(_uuid);
  _map.insert(std::make_pair("uuid", uuid));
  _map.insert(std::make_pair("api", SaiGetNet2Version()));
}

ProtMessage::~ProtMessage()
{
}

std::string 
ProtMessage::getUUID(std::string& ret)
{
  ret = boost::uuids::to_string(_uuid);
  return ret;
}

void 
ProtMessage::setId(uint32_t id)
{
  char cId[32];
  sprintf(cId, "%u", id);
  _map.insert(std::make_pair("id", cId));
}

void
ProtMessage::toString(std::string& ret)
{
  std::stringstream txt;
  _message.clear();
  uint16_t version = 1;
  version = htons(version);
  _message.append((char*)&version, sizeof(version));

  txt << "<?xml version=\"1.0\"?>\n";
  txt << "<protocol>";
  if (_map.size() > 0)
  {
    ProtMapIterator iter;
    for (iter  = _map.begin();
         iter != _map.end();
         iter ++)
    {
      txt << "<" << iter->first << " value=\"" << iter->second << "\"/>";
    }
  }
  txt << "</protocol>";
  _message.append(txt.str());
  ret = _message;
}

uint32_t 
ProtMessage::size()
{
  if (_message.size() == 0)
  {
    std::string dummy;
    toString(dummy);
  }
  return _message.size();
}

void 
DataDescriptor::print()
{
  switch(version)
  {
    case 2:
      printf("Version: 2\n");
      raw.r2.print();
      break;
  }
}

void 
Raw2::print()
{
  printf("    EncAlgo: %u\n", encAlgo);
  printf(" EncTagSize: %u\n", encTagSize);
  printf("   CompAlgo: %u\n", comAlgo);
  printf("CompTagSize: %u\n", comTagSize);
  printf("   HashAlgo: %u\n", hashAlgo);
  printf("HashTagSize: %u\n", hashTagSize);
  printf("   ProtSize: %u\n", protSize);
  printf("    XmlSize: %u\n", xmlSize);
  printf("    BinSize: %u\n", binSize);
  for (uint8_t i = 0; i < encTagSize; i += 1)
  {
    printf("%c", encTag[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint8_t i = 0; i < comTagSize; i += 1)
  {
    printf("%c", comTag[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint8_t i = 0; i < hashTagSize; i += 1)
  {
    printf("%02x", hashTag[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint32_t i = 0; i < protSize; i += 1)
  {
    printf("%c", protData[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint32_t i = 0; i < xmlSize; i += 1)
  {
    printf("%c", xmlData[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint32_t i = 0; i < binSize; i += 1)
  {
    printf("%c", binData[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
}

