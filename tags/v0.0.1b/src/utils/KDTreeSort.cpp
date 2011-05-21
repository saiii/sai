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
#include "KDTreeSort.h"

using namespace sai::utils;

namespace sai
{
namespace utils
{

class ParentNode : public KDTree::Node
{
  public:
    index_t        dim;
    index_t        count;
    Element      * aggregate;
    Element      * average;
    KDTree::Node * left;
    KDTree::Node * right;
    KDTree::Node * parent;

  public:
    ParentNode() : 
      dim(0), count(0), 
      aggregate(0), 
      average(0), left(0),
      right(0), parent(0) 
    {}
    ~ParentNode() 
    {
      delete aggregate;
      delete average;
    }
    bool     isLeaf()   { return false;   }
    Node*    getLeft()  { return left;    }
    Node*    getRight() { return right;   }
    Node*    getParent(){ return parent;  }
    Element* getValue() { return average; }
};

class LeafNode : public KDTree::Node
{
  public:
    Element      * value;
    KDTree::Node * parent;

  public:
    LeafNode() : value(0), parent(0) {}
    ~LeafNode() {}
    bool     isLeaf()   { return true;   }
    Node*    getLeft()  { return 0;      } 
    Node*    getRight() { return 0;      } 
    Node*    getParent(){ return parent; }
    Element* getValue() { return value;  }
};

class ExpandSearchData
{
public:
    LeafNode * left;
    LeafNode * right;
};

}
}

KDTree::KDTree(index_t dim, KDTreeListSorter * sorter, KDTreeElementCombiner * comb):
  _dim(dim),
  _root(0),
  _sorter(sorter),
  _comb(comb),
  _expandSearch(0)
{
  _root = new ParentNode();
}

KDTree::~KDTree()
{
  delete _expandSearch;
  delete _root;
}

void 
KDTree::sort(List* list)
{
  if (!list || list->size() == 0) 
  {
    return;
  }

  _root = sort(list, 0, 0);
}

KDTree::Node*
KDTree::sort(List* list, index_t depth, Node* parent)
{
  if (list->size() == 1)
  {
    LeafNode * leaf = new LeafNode();
    leaf->parent = parent;
    leaf->value  = list->at(0); 
    delete list;
    return leaf;
  }

  ParentNode * pNode = new ParentNode();
  index_t axis = depth % _dim; 
  pNode->dim   = axis;

  // Sort data from the selected axis
  _sorter->sort(list, axis);

  // Find median
  index_t size = list->size();
  index_t mid  = 0;

  mid = (size % 2 == 0) ? (size / 2) - 1 : (size / 2);

  pNode->parent = parent;
  DLinkedList * left = new DLinkedList();
  DLinkedList * right= new DLinkedList();

  list->split(mid + 1, left, right);
  delete list;

  if (left->size() > 0)
  {
    pNode->left = sort(left, depth + 1, pNode);
  }

  if (right->size() > 0)
  {
    pNode->right = sort(right, depth + 1, pNode);
  }

  if (!pNode->isLeaf())
  {
    ParentNode * pL = dynamic_cast<ParentNode*>(pNode->left);
    ParentNode * pR = dynamic_cast<ParentNode*>(pNode->right);
    Element * left, *right;
    index_t   cLeft, cRight;

    left  = pL ? pL->aggregate : pNode->left->getValue();
    right = pR ? pR->aggregate : pNode->right->getValue();
    pNode->aggregate = _comb->createAggregate(left, right);

    cLeft = pL ? pL->count : 1;
    cRight= pR ? pR->count : 1;
    pNode->count = cLeft + cRight;

    pNode->average  = _comb->createAverage(pNode->aggregate, pNode->count);
  }

  return pNode;
}

Element* 
KDTree::search(Key* key, bool nearestSearch)
{
  Node * node = _root; 
  FindableKDTreeElement * fKey = dynamic_cast<FindableKDTreeElement*>(key);
  if (!fKey)
  {
    return 0;
  }

  do
  {
    ParentNode * pNode = dynamic_cast<ParentNode*>(node);
    if (pNode)
    {
      FindableKDTreeElement * elem = dynamic_cast<FindableKDTreeElement*>(pNode->average);
      if (elem && (elem->isGreaterThan(fKey, pNode->dim) || elem->isEqual(fKey, pNode->dim)))
      {
        node = node->getLeft(); 
      }
      else
      {
        node = node->getRight(); 
      }
    }
  } while(!node->isLeaf());

  if (nearestSearch)
  {
    return node->getValue(); 
  }

  if (node->getValue()->operator==(key))
  {
    return node->getValue();
  }
  else
  {
    return 0;
  }
}

void 
KDTree::traverse(KDTreeTraversalHandler* handler)
{
  preorder(handler, _root);
}

void  
KDTree::preorder(KDTreeTraversalHandler* handler, Node* node)
{
  if (node->isLeaf())
  {
    handler->visitLeaf(node);
  }
  else
  {
    preorder(handler, node->getLeft());
    preorder(handler, node->getRight());
    handler->visitParent(node);
  }
}

