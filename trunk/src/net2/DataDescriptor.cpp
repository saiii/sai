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
#include <Objbase.h>
#else
#include <arpa/inet.h>
#endif
#include <cstring>
#include <cstdio>
#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include "DataDescriptor.h"

using namespace sai::net2;

Raw::Raw():
  encAlgo(0),
  encTagSize(0),
  comAlgo(0),
  comTagSize(0),
  hashAlgo(0),
  hashTagSize(0),
  protSize(0),
  xmlSize(0),
  binSize(0),
  encTag(0),
  comTag(0),
  hashTag(0),
  protData(0),
  xmlData(0),
  binData(0)
{
#ifdef _WIN32
  encTag = (char*)::CoTaskMemAlloc(RAW2_ENCTAG_SIZE);
  comTag = (char*)::CoTaskMemAlloc(RAW2_COMTAG_SIZE);
  hashTag= (char*)::CoTaskMemAlloc(RAW2_HASHTAG_SIZE);
#else
  encTag = new char[RAW2_ENCTAG_SIZE];
  comTag = new char[RAW2_COMTAG_SIZE];
  hashTag= new char[RAW2_HASHTAG_SIZE];
#endif
}

Raw::~Raw()
{
#ifdef _WIN32
  ::CoTaskMemFree(hashTag);
  ::CoTaskMemFree(comTag);
  ::CoTaskMemFree(encTag);
#else
  delete [] hashTag;
  delete [] comTag;
  delete [] encTag;
#endif
}

DataDescriptor::DataDescriptor():
  version(2),
  raw(0),
  protMessage(0),
  endpoint(0)
{
  raw = new Raw();
  protMessage = new ProtMessage();
}

DataDescriptor::~DataDescriptor()
{
  delete protMessage;
  delete raw;
}

extern std::string SaiGetNet2Version();
boost::uuids::uuid ProtMessage::_uuid = boost::uuids::random_generator()();

ProtMessage::ProtMessage()
{
  const std::string uuid = boost::uuids::to_string(_uuid);
  ProtKeyValue * entry = new ProtKeyValue();
  entry->set("uuid", uuid);
  _list.push_back(entry);

  entry = new ProtKeyValue();
  entry->set("api", SaiGetNet2Version());
  _list.push_back(entry);

  entry = new ProtKeyValue();
  entry->set("destination", ".*");
  _list.push_back(entry);
}

void
clear(ProtList& list)
{
  while (list.size() > 0)
  {
    ProtKeyValue * e = list.front();
    list.erase(list.begin());
    delete e;
  }
}

ProtMessage::~ProtMessage()
{
  clear(_list);
}

void
ProtMessage::getUUID(std::string& ret)
{
  ret = boost::uuids::to_string(_uuid);
}

void 
ProtMessage::setId(uint32_t id)
{
  char cId[32];
  sprintf(cId, "%u", id);
  ProtKeyValue * entry = new ProtKeyValue();
  entry->set("id", cId);
  _list.push_back(entry);
}

uint32_t 
ProtMessage::getId()
{
  if (_list.size() <= 0)
  {
    return 0;
  }

  ProtListIterator iter;
  for(iter  = _list.begin();
      iter != _list.end();
      iter ++)
  {
    ProtKeyValue * entry = *iter;
    if (entry->key->compare("id") == 0)
    {
      uint32_t ret = atol(entry->value->c_str());
      return ret;
    }
  }
  return 0;
}

void 
ProtMessage::setDestination(std::string& uuid)
{
  ProtListIterator iter;
  for(iter  = _list.begin();
      iter != _list.end();
      iter ++)
  {
    ProtKeyValue * entry = *iter;
    if (entry->key->compare("destination") == 0)
    {
      entry->set("destination", uuid);
      return;
    }
  }
}

void
ProtMessage::getDestination(std::string& ret)
{
  ret.clear();
  ProtListIterator iter;
  for(iter  = _list.begin();
      iter != _list.end();
      iter ++)
  {
    ProtKeyValue * entry = *iter;
    if (entry->key->compare("destination") == 0)
    {
      ret = *(entry->value);
      return;
    }
  }
}

void 
ProtMessage::getNodeUUID(std::string& ret)
{
  if (_list.size() <= 0)
  {
    return;
  }

  ProtListIterator iter;
  for(iter  = _list.begin();
      iter != _list.end();
      iter ++)
  {
    ProtKeyValue * entry = *iter;
    if (entry->key->compare("uuid") == 0)
    {
      ret = entry->value->c_str();
      return;
    }
  }
}

void 
GetAndAdd(std::string tag, std::string attr, sai::utils::XmlReader& reader, ProtList& list)
{
  std::string value;
  reader.get(tag, attr, value);
  if (value.length() > 0)
  {
    ProtKeyValue * entry = new ProtKeyValue();
    entry->set(tag, value);
    list.push_back(entry); 
  }
}

void 
ProtMessage::fromString(std::string xml)
{
  clear(_list);

  sai::utils::XmlReader pReader;
  pReader.parseMem(xml);
  //printf("%s\n", xml.c_str());
  pReader.moveTo("protocol");

  ProtKeyValue * entry = 0;
  std::string uuid;
  pReader.get("uuid","value", uuid);
  entry = new ProtKeyValue();
  entry->set("uuid", uuid); _list.push_back(entry); 

  std::string sid;
  pReader.get("id",  "value", sid);
  entry = new ProtKeyValue();
  entry->set("id", sid); _list.push_back(entry); 

  std::string api;
  pReader.get("api", "value", api);
  entry = new ProtKeyValue();
  entry->set("api", api); _list.push_back(entry); 

  std::string destination;
  pReader.get("destination", "value", destination);
  entry = new ProtKeyValue();
  entry->set("destination", destination); _list.push_back(entry); 

  GetAndAdd("loginserver", "value", pReader, _list);
  GetAndAdd("loginport",   "value", pReader, _list);
  GetAndAdd("username", "value", pReader, _list);
  GetAndAdd("password", "value", pReader, _list);
  GetAndAdd("result", "value", pReader, _list);
  GetAndAdd("session_key", "value", pReader, _list);
  GetAndAdd("session_iv", "value", pReader, _list);
}

void 
ProtMessage::set(std::string name, std::string value)
{
  if (_list.size() <= 0)
  {
    return;
  }

  ProtListIterator iter;
  for(iter  = _list.begin();
      iter != _list.end();
      iter ++)
  {
    ProtKeyValue * entry = *iter;
    if (entry->key->compare(name) == 0)
    {
      entry->value->assign(value);
      return;
    }
  }
  ProtKeyValue * entry = new ProtKeyValue();
  entry->set(name, value);
  _list.push_back(entry);
}

void
ProtMessage::get(const char* name, std::string& value)
{
  value.clear();
  if (_list.size() <= 0)
  {
    return;
  }

  ProtListIterator iter;
  for(iter  = _list.begin();
      iter != _list.end();
      iter ++)
  {
    ProtKeyValue * entry = *iter;
    if (entry->key->compare(name) == 0)
    {
      value.append(*entry->value);
      return;
    }
  }
}

void
ProtMessage::toString(std::string& ret, uint32_t& size)
{
  std::stringstream txt;
  ret.clear();
  uint16_t version = 1;
  version = htons(version);
  ret.append((char*)&version, sizeof(version));

  txt << "<?xml version=\"1.0\"?>\n";
  txt << "<protocol>";
  if (_list.size() > 0)
  {
    ProtListIterator iter;
    for (iter  = _list.begin();
         iter != _list.end();
         iter ++)
    {
      txt << "<" << (*iter)->key->c_str() << " value=\"" << (*iter)->value->c_str() << "\"/>";
    }
  }
  txt << "</protocol>";
  ret.append(txt.str());
  size = ret.size();
}

void 
DataDescriptor::print()
{
  switch(version)
  {
    case 2:
      printf("Version: 2\n");
      raw->print();
      break;
    case 3:
      printf("Version: 3\n");
      raw->print();
      break;
  }
}

void 
Raw::print()
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
  for (uint16_t i = 0; i < encTagSize; i += 1)
  {
    printf("%c", encTag[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint16_t i = 0; i < comTagSize; i += 1)
  {
    printf("%c", comTag[i]);
    if ((i+1)%70 == 0) { printf("\n"); }
  }
  printf("\n");
  for (uint16_t i = 0; i < hashTagSize; i += 1)
  {
    printf("%02x", (uint8_t)hashTag[i]);
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

ProtKeyValue::ProtKeyValue():
  key(0),
  value(0)
{
  key = new std::string();
  value = new std::string();
}

ProtKeyValue::~ProtKeyValue()
{
  delete key;
  delete value;
}

void 
DataDescriptor::HashSAI64(char * msg, uint32_t size, uint64_t& value)
{
  char * start = msg;
  char * end = msg + (size > 100 ? 100 : size);
  uint64_t magic = 0xFFFFFFFFFFFFFFFFLL;
  value = 0;
  for (char * ptr = start; ptr < end; ptr++)
  {
    char ch = *ptr;
    value += (magic | ch);
    magic <<= 1;
    if (magic == 0) magic = 1;
  }
}

