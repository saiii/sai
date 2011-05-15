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

#ifndef __SAI_UTILS_LIST__
#define __SAI_UTILS_LIST__

#include <utils/Types.h>

namespace sai
{
namespace utils
{

class Sorter;
class Element;
class List
{
public:
  List();
  virtual ~List();

  virtual index_t  size() = 0;
  virtual void     split(index_t pos, List *left, List *right) = 0;
  virtual Element* at(index_t) = 0;
  virtual Element* operator[](index_t) = 0;
  virtual void     insert(index_t, Element*) = 0;
  virtual void     append(Element*) = 0;
  virtual void     remove(index_t) = 0;
  virtual void     swap(index_t, index_t) = 0;
};

// Doubly Linked List
class _DLNode;
class DLinkedList : public List
{
private:
  index_t  _size;
  _DLNode* _terminator;

public:
  DLinkedList();
  virtual ~DLinkedList();

  virtual index_t  size();
  virtual void     split(index_t, List*, List*);
  virtual Element* at(index_t);
  virtual Element* operator[](index_t);
  virtual void     insert(index_t, Element*);
  virtual void     append(Element*);
  virtual void     remove(index_t);
  virtual void     swap(index_t, index_t);
};

}
}

#endif
