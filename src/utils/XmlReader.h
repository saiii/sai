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

#ifndef __SAI_UTILS_XMLREADER__
#define __SAI_UTILS_XMLREADER__

#include <stdint.h>
#include <string>
#include <vector>

namespace sai
{
namespace utils
{

class Pair
{
public:
  std::string name;
  std::string value;
};

typedef std::vector<Pair*>           PairList;
typedef std::vector<Pair*>::iterator PairListIterator;

class XmlReaderImpl;
class XmlReader
{
private:
  XmlReaderImpl* _impl;

public:
  XmlReader();
  virtual ~XmlReader();

  void parseFile(std::string xmlFile);
  void parseMem(std::string xmlMessage);
  void moveTo(std::string tag);
  void get(std::string tag, std::string attribute, std::string& ret);

  uint32_t count();
  void     getChild(uint32_t index, std::string& name, PairList& attributeList);

  static std::string& EncodeSpecialCharacter(std::string& from);
};


}}
#endif
