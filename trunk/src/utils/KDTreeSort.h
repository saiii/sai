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

#ifndef __SAI_UTILS_KDTREESORT__
#define __SAI_UTILS_KDTREESORT__

#include <utils/Sorter.h>
#include <utils/Searcher.h>

namespace sai
{
namespace utils
{

class KDTreeListSorter
{
public:
  virtual void sort(List*, index_t dim) = 0;
};

class KDTreeElementCombiner
{
public:
  virtual Element* createAggregate(Element*, Element*) = 0;
  virtual Element* createAverage(Element*, index_t) = 0;
};

class FindableKDTreeElement
{
public:
  virtual bool isGreaterThan(FindableKDTreeElement*,index_t) = 0;
  virtual bool isLessThan(FindableKDTreeElement*,index_t) = 0;
  virtual bool isEqual(FindableKDTreeElement*,index_t) = 0;
};

class List;
class Element;
class KDTreeTraversalHandler;
class ExpandSearchData;
class KDTree : public Sorter,
               public Searcher
{
public:
  class Node
  {
    public:
      virtual bool     isLeaf()   = 0;
      virtual Node*    getLeft()  = 0;
      virtual Node*    getRight() = 0;
      virtual Node*    getParent()= 0;
      virtual Element* getValue() = 0;
  };

private:
  index_t                 _dim;
  Node                  * _root;
  KDTreeListSorter      * _sorter;
  KDTreeElementCombiner * _comb;
  ExpandSearchData      * _expandSearch;

private:
  Node* sort(List*, index_t, Node*);
  void  preorder(KDTreeTraversalHandler*, Node*);

public:
  KDTree(index_t, KDTreeListSorter *, KDTreeElementCombiner*);
  ~KDTree();

  void     sort(List* list, Comparer*) { sort(list); } // Note: The given list will be destroyed
  void     sort(List*); 
  Element* search(List*, Key* key) { return search(key, false); }
  Element* search(Key* key)        { return search(key, false); }
  Element* search(Key*,bool);
  Element* nearestSearch(Key* key) { return search(key, true);  }
  Element* expandSearch();
  void     traverse(KDTreeTraversalHandler*); // only preorder for now
};

class KDTreeTraversalHandler
{
public:
  virtual void visitLeaf(KDTree::Node*)   = 0;
  virtual void visitParent(KDTree::Node*) = 0;
};


}
}

#endif
