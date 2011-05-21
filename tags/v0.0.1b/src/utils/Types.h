//=============================================================================
// Copyright (C) 2010 Athip Rooprayochsilp <athipr@gmail.com>
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

#ifndef __SAI_UTILS_TYPES__
#define __SAI_UTILS_TYPES__

#include <string>

namespace sai
{
namespace utils
{

class Key
{
public:
  Key() {}
  virtual ~Key(){}
};

class Element
{
public:
  virtual bool operator> (Element*) = 0;
  virtual bool operator< (Element*) = 0;
  virtual bool operator==(Key*)     = 0;
  virtual bool operator> (Key*)  = 0;
  virtual bool operator< (Key*)  = 0;
  virtual std::string toString() = 0; // TO BE REMOVED
};

class Comparer
{
public:
  virtual bool compare(Element*, Element*) = 0;
};

typedef unsigned int index_t;

}
}

#endif
