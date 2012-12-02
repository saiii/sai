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

#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdint.h>
#include <cstdio>
#include <utils/rapidxml.hpp>
#include <utils/XmlReader.h>

using namespace rapidxml;


namespace sai { namespace utils {

class XmlReaderImpl
{
public:
  xml_document<>            doc;
  xml_node<>               *root;
  xml_node<>               *startNode;
  char                     *buffer;

  std::string               xmlData;
  std::string               ret;

public:
  XmlReaderImpl() :  
    root(0), 
    startNode(0),
    buffer(0)
  {}
  ~XmlReaderImpl()
  {
    delete [] buffer;
  }
};
}}

using namespace sai::utils;

XmlReader::XmlReader():
  _impl(0)
{
  _impl = new XmlReaderImpl();
}

XmlReader::~XmlReader()
{
  delete _impl;
}

void 
XmlReader::parseFile(std::string xmlFile)
{
  std::string xmlData;

#ifdef _WIN32
  TCHAR fName[1024];
  for (unsigned int index = 0; index < xmlFile.length(); index += 1)
  {
    fName[index]   = (TCHAR) xmlFile.at(index);
    fName[index+1] = 0;
  }

  HANDLE hFile = CreateFile(fName, 
                   GENERIC_READ, 
                   FILE_SHARE_READ, 
                   NULL, 
                   OPEN_EXISTING, 
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    return;
  }
#else
  FILE * fp = fopen(xmlFile.c_str(), "r");
  if (fp == NULL)
  {
    return;
  }
#endif

  char buffer[1024];
#ifdef _WIN32
  DWORD bytes = 0;
  do
  {
    ReadFile(hFile, buffer, sizeof(buffer), &bytes, NULL);
    xmlData.append(buffer, bytes);
  }while(bytes >= sizeof(buffer));
  CloseHandle(hFile);
#else
  size_t bytes = 0;
  do
  {
    bytes = fread(buffer, 1, sizeof(buffer), fp);
    xmlData.append(buffer, bytes);
  }while(bytes >= sizeof(buffer));
  fclose(fp);
#endif

  parseMem(xmlData);
}

void 
XmlReader::parseMem(std::string xmlMessage)
{
  _impl->buffer = new char[xmlMessage.length()+1];
  memset(_impl->buffer, 0, xmlMessage.length() + 1);
  memcpy(_impl->buffer, xmlMessage.c_str(), xmlMessage.length());
  _impl->doc.parse<0>(_impl->buffer);

  _impl->xmlData.clear();
  _impl->xmlData.append(xmlMessage.c_str(), xmlMessage.length());

  _impl->root = _impl->doc.first_node();
}

xml_node<>*
find(xml_node<>* element, std::string id)
{
  char * name = element->name();
  if (id.compare(name) == 0)
  {
    return element;
  }

  xml_node<>* children = element->first_node();

  if (!children)
  {
    return 0;
  }
  else
  {
    xml_node<>* rtn = 0;
    for(; children; children= children->next_sibling())
    {
      if ((rtn = find(children, id)) != 0)
        return rtn;
    }
  }

  return 0;
}

void
getAttribute(xml_node<>* element, std::string name, std::string& value)
{
  value.clear();

  xml_attribute<> * attribute = element->first_attribute(name.c_str());
  if (!attribute)
  {
    return;
  }

  value = attribute->value();
}

uint32_t 
XmlReader::count()
{
  xml_node<> * element = _impl->startNode ? _impl->startNode : _impl->root;
  uint32_t ret = 0;
  for(xml_node<>* children = element->first_node(); children; children = children->next_sibling())
  {
    ret += 1;
  }
  return ret;
}

void     
XmlReader::getChild(uint32_t index, std::string& name, PairList& attributeList)
{
  abort();
}

void
XmlReader::get(std::string tag, std::string attribute, std::string& ret)
{
  if (!_impl->root) 
  {
    return;
  }

  _impl->ret.clear();

  xml_node<>* elem = 0;
  elem = find(_impl->startNode ? _impl->startNode : _impl->root, tag);
  if (elem)
  {
    getAttribute(elem, attribute, _impl->ret);
  }
  ret = _impl->ret;
}

void  
XmlReader::moveTo(std::string tag)
{
  _impl->startNode = find(_impl->root, tag);
}

std::string& 
XmlReader::EncodeSpecialCharacter(std::string& from)
{
  std::string n;
  for (uint32_t i = 0; i < from.length(); i += 1)
  {
    switch (from.at(i))
    {
      case '&':
        n.append("&amp;");
        break;
      case '<':
        n.append("&lt;");
        break;
      case '>':
        n.append("&gt;");
        break;
      case '"':
        n.append("&quot;");
        break;
      case '\'':
        n.append("&#39;");
        break;
      default:
        {
          char tmp[2] = {from.at(i), 0};
          n.append(tmp);
        }
        break;
    }
  }

  from = n;
  return from;
}
