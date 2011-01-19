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

#include "List.h"

using namespace sai::utils;

List::List()
{}

List::~List()
{}

namespace sai { namespace utils {

class _DLNode
{
friend class DLinkedList;
public:
  _DLNode* next;
  _DLNode* prev;
  Element* value;

private:
  _DLNode() : next(0), prev(0), value(0) {}
  ~_DLNode() {}
};

}}

DLinkedList::DLinkedList():
  _size(0),
  _terminator(0)
{
  _terminator = new _DLNode();
  _terminator->next = _terminator;
  _terminator->prev = _terminator;
}

DLinkedList::~DLinkedList()
{
  delete _terminator;
}

index_t  
DLinkedList::size()
{
  return _size;
}

void 
DLinkedList::split(index_t pos, List* left, List* right)
{
  _DLNode * node = _terminator->next;
  for (index_t i = 0; i < pos; i += 1)
  {
    node = node->next;
  }

  DLinkedList * dleft  = dynamic_cast<DLinkedList*>(left);
  DLinkedList * dright = dynamic_cast<DLinkedList*>(right);

  _DLNode * lastLeft   = node->prev;
  _DLNode * firstRight = node;
  _DLNode * first = _terminator->next;
  _DLNode * last  = _terminator->prev;

  dleft->_terminator->next = first;
  dleft->_terminator->prev = lastLeft;
  first->prev              = dleft->_terminator;
  lastLeft->next           = dleft->_terminator;
  dleft->_size             = pos;

  dright->_terminator->next = firstRight;
  dright->_terminator->prev = last;
  firstRight->prev          = dright->_terminator;
  last->next                = dright->_terminator;
  dright->_size             = _size - pos;

  _terminator->next = _terminator;
  _terminator->prev = _terminator;
  _size = 0;
}

Element* 
DLinkedList::at(index_t pos)
{
  _DLNode * node = _terminator->next;
  for (index_t i = 0; i < pos; i += 1)
  {
    node = node->next;
  }

  return node->value;
}

Element* 
DLinkedList::operator[](index_t pos)
{
  _DLNode * node = _terminator->next;
  for (index_t i = 0; i < pos; i += 1)
  {
    node = node->next;
  }

  return node->value;
}

void 
DLinkedList::insert(index_t pos, Element* elem)
{
  _DLNode * node = _terminator->next;
  for (index_t i = 0; i < pos; i += 1)
  {
    node = node->next;
  }

  _DLNode * nnode = new _DLNode(); 
  nnode->value = elem;

  _DLNode * left  = node->prev;
  _DLNode * right = node;
  nnode->next = right;
  nnode->prev = left;

  left->next = nnode;
  right->prev= nnode;

  _size += 1;
}

void 
DLinkedList::append(Element* elem)
{
  _DLNode * left = _terminator->prev;
  _DLNode * right= _terminator;

  _DLNode * node = new _DLNode();
  node->value = elem;

  node->next = right;
  node->prev = left;

  left->next = node;
  right->prev= node;

  _size += 1;
}

void 
DLinkedList::remove(index_t pos)
{
  _DLNode * node = _terminator->next;
  for (index_t i = 0; i < pos; i += 1)
  {
    node = node->next;
  }

  if (node == _terminator)
  { 
    return;
  }

  _DLNode * left = node->prev;
  _DLNode * right= node->next;

  left->next = right;
  right->prev= left;

  delete node;
  _size -= 1;
}

void 
DLinkedList::swap(index_t i, index_t j)
{
  _DLNode * nodei = _terminator->next;
  for (index_t ind = 0; ind < i; ind += 1)
  {
    nodei = nodei->next;
  }

  _DLNode * nodej = _terminator->next;
  for (index_t ind = 0; ind < j; ind += 1)
  {
    nodej = nodej->next;
  }

  Element * tmp = nodei->value;
  nodei->value = nodej->value;
  nodej->value = tmp;
}
