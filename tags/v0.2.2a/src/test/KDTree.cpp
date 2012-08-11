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

#include <iostream>
#include <sstream>
#include <string>
#include <utils/Types.h>
#include <utils/BubbleSort.h>
#include <utils/BinarySearch.h>
#include <utils/LinearSearch.h>
#include <utils/List.h>
#include <utils/KDTreeSort.h>

using namespace sai::utils;

void kdtree();

int main(int argc, char * argv[])
{
  kdtree();
  return 0;
}

class KDTreeElement : public Element, public FindableKDTreeElement, public Key
{
  public:
    double data[2];

  public:
    KDTreeElement(double v1, double v2) { data[0] = v1; data[1] = v2; }
    ~KDTreeElement() {}
    bool operator> (Element*) { return false; }
    bool operator< (Element*) { return false; }
    bool operator==(Key* key) { KDTreeElement * elem = dynamic_cast<KDTreeElement*>(key); 
                                return (elem && elem->data[0] == data[0] && elem->data[1] == data[1])? true : false; }
    bool operator> (Key*)     { return false; }
    bool operator< (Key*)     { return false; }

    bool isGreaterThan(FindableKDTreeElement* e, index_t i) 
                              { KDTreeElement * elem = dynamic_cast<KDTreeElement*>(e); 
			        return (elem && data[i] > elem->data[i]) ? true : false; }
    bool isLessThan(FindableKDTreeElement* e, index_t i) 
                              { KDTreeElement * elem = dynamic_cast<KDTreeElement*>(e); 
			        return (elem && data[i] < elem->data[i]) ? true : false; }
    bool isEqual(FindableKDTreeElement* e, index_t i) 
                              { KDTreeElement * elem = dynamic_cast<KDTreeElement*>(e); 
			        return (elem && data[i] == elem->data[i]) ? true : false; }
    std::string toString() 
    { 
      static std::string ret = ""; 
      std::stringstream str;
      ret.clear(); 
      str << "(" << data[0] << ", " << data[1] << ")";
      ret = str.str();
      return ret; 
    }
};

class MyListSorter : public KDTreeListSorter
{
  private:
    class Comp : public Comparer
    {
      public:
        index_t dim;

      public:
        Comp():dim(0) {}
	~Comp() {}

        bool compare(Element* a, Element* b)
	{
          KDTreeElement * ka = dynamic_cast<KDTreeElement*>(a);
          KDTreeElement * kb = dynamic_cast<KDTreeElement*>(b);
          return (ka->data[dim] > kb->data[dim]);
	}
    };

    Sorter * sorter;
    Comp   * comp;

  public:
    MyListSorter() { sorter = new BubbleSort(); comp = new Comp(); }
    ~MyListSorter(){ delete sorter; delete comp; }

    void sort(List* list, index_t dim)
    {
      comp->dim = dim;
      sorter->sort(list, comp);
    }
};

class MyTraversalHandler : public KDTreeTraversalHandler
{
public:
  void visitLeaf(KDTree::Node * node)
  {
    Element * elem = node->getValue();
    std::cout << "Leaf " << elem->toString().c_str() << std::endl;
  }

  void visitParent(KDTree::Node * node)
  {
    Element * elem = node->getValue();
    std::cout << "Parent " << elem->toString().c_str() << std::endl;
  }
};

class MyDoubleCombiner : public KDTreeElementCombiner
{
public:
  Element* createAggregate(Element* a, Element* b) 
  {
    KDTreeElement * dA = dynamic_cast<KDTreeElement*>(a);
    KDTreeElement * dB = dynamic_cast<KDTreeElement*>(b);

    double ret[2] = {0.0, 0.0};
    if (dA && dB)
    {
      ret[0] = (dA->data[0] + dB->data[0]);
      ret[1] = (dA->data[1] + dB->data[1]);
    }
    else if (dA)
    {
      ret[0] = (dA->data[0]);
      ret[1] = (dA->data[1]);
    }
    else if (dB)
    {
      ret[0] = (dB->data[0]);
      ret[1] = (dB->data[1]);
    }
    return new KDTreeElement(ret[0], ret[1]);
  }

  Element* createAverage(Element* a, index_t count)
  {
    KDTreeElement * dA = dynamic_cast<KDTreeElement*>(a);

    double ret[2] = {0.0, 0.0};
    if (dA)
    {
      ret[0] = dA->data[0] / count;
      ret[1] = dA->data[1] / count;
    }
    return new KDTreeElement(ret[0], ret[1]);
  }
};

void 
kdtree()
{
  List * list = new DLinkedList();
  list->append(new KDTreeElement(3, 3));
  list->append(new KDTreeElement(2, 4));
  list->append(new KDTreeElement(1, 5));
  list->append(new KDTreeElement(5, 1));
  list->append(new KDTreeElement(4, 2));

  MyListSorter * lsorter = new MyListSorter();
  MyDoubleCombiner * comb = new MyDoubleCombiner();
  KDTree* sorter = new KDTree(2, lsorter, comb);
  MyTraversalHandler * handler = new MyTraversalHandler();

  KDTreeElement key1(3, 3);

  sorter->sort(list, 0);
  sorter->traverse(handler); 

  Element * elem = sorter->search(&key1);
  if (elem)
  {
    std::cout << "Found " << elem->toString().c_str() << std::endl;
  }
  else
  {
    std::cout << "NOTFound!! " << std::endl;
  }

  delete handler;
  delete comb;
  delete lsorter;
  delete sorter;
}

