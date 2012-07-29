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
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include <utils/XmlReader.h>

namespace sai { namespace utils {

class XmlReaderImpl
{
public:
  xercesc::XercesDOMParser *parser;
  xercesc::DOMElement      *root;
  xercesc::DOMElement      *startNode;
  std::string               xmlData;
  std::string               ret;

public:
  XmlReaderImpl() :  
    parser(0),
    root(0),
    startNode(0)
  {}
  ~XmlReaderImpl()
  {}
};
}}

using namespace sai::utils;

XmlReader::XmlReader():
  _impl(0)
{
  _impl = new XmlReaderImpl();

  try
  {
    xercesc::XMLPlatformUtils::Initialize();
    _impl->parser = new xercesc::XercesDOMParser();
  }
  catch (const xercesc::XMLException& e)
  {
  }
}

XmlReader::~XmlReader()
{
  delete _impl->parser;
  delete _impl;
  xercesc::XMLPlatformUtils::Terminate();
}

void 
XmlReader::parseFile(std::string xmlFile)
{
  _impl->parser->setValidationScheme(xercesc::XercesDOMParser::Val_Never);
  _impl->parser->setDoNamespaces(false);
  _impl->parser->setDoSchema(false);
  _impl->parser->setLoadExternalDTD(false);

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
  _impl->parser->setValidationScheme(xercesc::XercesDOMParser::Val_Never);
  _impl->parser->setDoNamespaces(false);
  _impl->parser->setDoSchema(false);
  _impl->parser->setLoadExternalDTD(false);

  _impl->xmlData = xmlMessage;
  try
  {
     xercesc::MemBufInputSource memBuf((const XMLByte*) _impl->xmlData.c_str(),
                                      _impl->xmlData.size(), "dummy", false);
    _impl->parser->parse(memBuf);

    if (_impl->parser->getErrorCount() > 0)
    {
      return;
    }
 
    xercesc::DOMDocument * xmlDoc = _impl->parser->getDocument();
    _impl->root = xmlDoc->getDocumentElement();
  }
  catch(xercesc::XMLException& e)
  {
  }
}

xercesc::DOMElement*
find(xercesc::DOMElement* element, std::string id)
{
  const XMLCh * tagName = element->getTagName();
  char * name = xercesc::XMLString::transcode(tagName);
  if (id.compare(name) == 0)
  {
    xercesc::XMLString::release(&name);
    return element;
  }
  xercesc::XMLString::release(&name);

  xercesc::DOMNodeList * children = element->getChildNodes();
  const XMLSize_t nodeCount = children->getLength();

  if (nodeCount <= 0)
  {
    return 0;
  }
  else
  {
    for(XMLSize_t i = 0; i < nodeCount; i += 1)
    {
      xercesc::DOMNode * currentNode = children->item(i);

      if (!currentNode || currentNode->getNodeType() != xercesc::DOMNode::ELEMENT_NODE)
        continue;

      xercesc::DOMElement * currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);
      xercesc::DOMElement * rtn = 0;
      if ((rtn = find(currentElement, id)) != 0)
        return rtn;
    }
  }

  return 0;
}

std::string
getAttribute(xercesc::DOMElement * element, std::string name)
{
  static std::string rtn;
  XMLCh * value = xercesc::XMLString::transcode(name.c_str());
  if (element->hasAttribute(value))
  {
    const XMLCh * ret = element->getAttribute(value);
    char * tmp = xercesc::XMLString::transcode(ret);
    rtn = tmp;
    xercesc::XMLString::release(&tmp);
  }
  xercesc::XMLString::release(&value);
  return rtn;
}

uint32_t 
XmlReader::count()
{
  xercesc::DOMElement* element = _impl->startNode ? _impl->startNode : _impl->root;
  xercesc::DOMNodeList * children = element->getChildNodes();
  return children->getLength();
}

void     
XmlReader::getChild(uint32_t index, std::string& name, PairList& attributeList)
{
  name.clear();
  attributeList.clear();

  xercesc::DOMElement* element = _impl->startNode ? _impl->startNode : _impl->root;
  xercesc::DOMNodeList * children = element->getChildNodes();
  if (index >= children->getLength()) return;

  xercesc::DOMNode * currentNode = children->item(index);

  if (!currentNode || currentNode->getNodeType() != xercesc::DOMNode::ELEMENT_NODE)
    return;

  xercesc::DOMNamedNodeMap* map = currentNode->getAttributes();
  for (uint32_t i = 0; i < map->getLength(); i += 1)
  {
    xercesc::DOMNode * item = map->item(i);

    const XMLCh * nodeName = element->getNodeName();
    const XMLCh * nodeValue= element->getNodeValue();

    char * sname = xercesc::XMLString::transcode(nodeName);
    char * svalue= xercesc::XMLString::transcode(nodeValue);
    Pair * pair = new Pair();
    pair->name.assign(sname);
    pair->value.assign(svalue);
    attributeList.push_back(pair);
    xercesc::XMLString::release(&sname);
    xercesc::XMLString::release(&svalue);
  }
}

std::string 
XmlReader::get(std::string tag, std::string attribute)
{
  _impl->ret.clear();

  xercesc::DOMElement * elem = 0;
  elem = find(_impl->startNode ? _impl->startNode : _impl->root, tag);
  if (elem)
  {
    _impl->ret = getAttribute(elem, attribute);
  }
  return _impl->ret;
}

void  
XmlReader::moveTo(std::string tag)
{
  xercesc::DOMElement * elem = 0;
  xercesc::DOMNodeList * children = _impl->root->getChildNodes();
  const XMLSize_t nodeCount = children->getLength();
  for(XMLSize_t i = 0; i < nodeCount; i += 1)
  {
    xercesc::DOMNode * currentNode = children->item(i);

    if (!currentNode)
      continue;

    if(xercesc::DOMNode::ELEMENT_NODE != currentNode->getNodeType())
      continue;

    xercesc::DOMElement * currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);
    if ((elem = find(currentElement, tag)) != 0)
      break;
  }

  _impl->startNode = elem;
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
