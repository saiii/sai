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

#include <iostream>
#include <sstream>
#include <string>
#include <utils/Types.h>
#include <utils/BubbleSort.h>
#include <utils/BinarySearch.h>
#include <utils/LinearSearch.h>
#include <utils/List.h>

using namespace sai::utils;

class Double : public Element, 
	       public Key
{
friend class MyDoubleCombiner;
private:
  double _i;

public:
  Double(double i):_i(i){}
  ~Double(){}
  bool operator< (Element*);
  bool operator> (Element*);
  bool operator==(Key*);
  bool operator< (Key*);
  bool operator> (Key*);
  std::string toString();
};

class Comp : public Comparer
{
public:
  bool compare(Element*, Element*);
};

void print(List*);
void print(Element *);

void list();
void simple_01();
void simple_02();

int main(int argc, char * argv[])
{
  list();
  simple_01();
  simple_02();
  return 0;
}

#define PRINT_ALL(a) { for (index_t i = 0; i < a->size(); i += 1) { \
	                 std::cout << "[" << i << "] = " << a->at(i)->toString().c_str() << std::endl; \
                         } }

void
list()
{
  std::cout << "" << std::endl;

  DLinkedList list;
  std::cout << "append(1)" << std::endl;
  list.append(new Double(1));
  std::cout << "List.size = " << list.size() << std::endl;
  std::cout << "List[0] = " << list.at(0)->toString().c_str() << std::endl;
  std::cout << "List[0] = " << list[0]->toString().c_str() << std::endl;

  std::cout << "insert(0, '2')" << std::endl;
  list.insert(0, new Double(2));
  std::cout << "List.size = " << list.size() << std::endl;
  std::cout << "List[0] = " << list[0]->toString().c_str() << std::endl;

  std::cout << "insert(1, '3')" << std::endl;
  list.insert(1, new Double(3));
  std::cout << "List.size = " << list.size() << std::endl;
  std::cout << "List[0] = " << list[0]->toString().c_str() << std::endl;

  std::cout << "remove(0)" << std::endl;
  list.remove(0);
  std::cout << "List.size = " << list.size() << std::endl;
  std::cout << "List[0] = " << list[0]->toString().c_str() << std::endl;

  std::cout << "split" << std::endl;
  list.append(new Double(4));
  list.append(new Double(5));
  list.append(new Double(6));
  DLinkedList * listPtr = &list;
  PRINT_ALL(listPtr);

  DLinkedList * left = new DLinkedList();
  DLinkedList * right= new DLinkedList();
  list.split(3, left, right);
  std::cout << "List.size = " << list.size() << std::endl;
  std::cout << "Left.size  = " << left->size()  << std::endl;
  std::cout << "Right.size = " << right->size() << std::endl;

  PRINT_ALL(left);
  PRINT_ALL(right);

  delete left;
  delete right;
}

void 
simple_01()
{
  Sorter * sorter = 0;
  Comparer * comp = 0;
  Searcher * search = 0;

  DLinkedList* list = new DLinkedList();
  list->append(new Double(7));
  list->append(new Double(3));
  list->append(new Double(2));
  list->append(new Double(9));
  const index_t listSize = 4;

  sorter = new BubbleSort();
  comp   = new Comp();
  search = new BinarySearch();
  //search = new LinearSearch();
  sorter->sort(list, comp);
  print(list);

  // find all elements in list
  int fList [] = {2, 3, 7, 9};
  for (index_t i = 0; i < listSize; i += 1)
  {
    std::cout << "FIND " << fList[i] << std::endl;
    Key * k = new Double(fList[i]);
    print(search->search(list, k));
    delete k;
  }

  delete sorter;
  delete comp;
  delete search;

  for (index_t i = 0; i < listSize; i +=1)
  {
    delete list->at(i);
  }
}

void 
simple_02()
{
  Sorter * sorter = 0;
  Comparer * comp = 0;
  Searcher * search = 0;

  List * list = new DLinkedList();
  list->append(new Double(7));
  list->append(new Double(3));
  list->append(new Double(2));
  list->append(new Double(9));
  list->append(new Double(6));
  const index_t listSize = 5;

  sorter = new BubbleSort();
  comp   = new Comp();
  search = new BinarySearch();
  //search = new LinearSearch();
  sorter->sort(list, comp);
  print(list);

  // find all elements in list
  int fList [] = {2, 3, 6, 7, 9};
  for (index_t i = 0; i < listSize; i += 1)
  {
    std::cout << "FIND " << fList[i] << std::endl;
    Key * k = new Double(fList[i]);
    print(search->search(list, k));
    delete k;
  }

  delete sorter;
  delete comp;
  delete search;

  for (index_t i = 0; i < listSize; i +=1)
  {
    delete list->at(i);
  }
}

void 
print(List *list)
{
  std::cout << "Size = " << list->size() << std::endl;
  for (index_t i = 0; i < list->size(); i += 1)
  {
    Double * elem = dynamic_cast<Double*>(list->at(i));
    std::cout << "Element[" << i << "] = " << elem->toString().c_str() << std::endl;
  }
}

void
print(Element * elem)
{
  Double * e = dynamic_cast<Double*>(elem);
  if (e == 0)
    std::cout << "Element = null" << std::endl;
  else
    std::cout << "Element = " << e->toString().c_str() << std::endl;
}

bool 
Double::operator< (Element* elem)
{
  Double * i = dynamic_cast<Double*>(elem);
  return (_i < i->_i);
}

bool 
Double::operator> (Element* elem)
{
  Double * i = dynamic_cast<Double*>(elem);
  return (_i > i->_i);
}

bool 
Double::operator==(Key* key)
{
  Double * k = dynamic_cast<Double*>(key);
  return (k->_i == _i);
}

bool 
Double::operator< (Key* key)
{
  Double * k = dynamic_cast<Double*>(key);
  return (_i < k->_i);
}

bool 
Double::operator> (Key* key)
{
  Double * k = dynamic_cast<Double*>(key);
  return (_i > k->_i);
}

std::string
Double::toString()
{
  static std::string ret;
  std::stringstream str;
  ret.clear();
  str << _i;
  ret = str.str();
  return ret;
}

bool 
Comp::compare(Element* a, Element* b)
{
  return (*a > b);
}

