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
    KDTree::Node * left;
    KDTree::Node * right;
    KDTree::Node * parent;

  public:
    ParentNode() : left(0),right(0) {}
    ~ParentNode() {}
    bool     isLeaf()  { return false; }
    Node*    getLeft() { return left;  }
    Node*    getRight(){ return right; }
    Element* getValue(){ return 0;     }
};

class LeafNode : public KDTree::Node
{
  public:
    Element      * value;
    KDTree::Node * parent;

  public:
    LeafNode() : value(0), parent(0) {}
    ~LeafNode() {}
    bool     isLeaf()  { return true; }
    Node*    getLeft() { return 0;    } 
    Node*    getRight(){ return 0;    } 
    Element* getValue(){ return value;}
};

}
}

KDTree::KDTree(index_t dim, ListSorter * sorter):
  _dim(dim),
  _root(0),
  _sorter(sorter)
{
  _root = new ParentNode();
}

KDTree::~KDTree()
{
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

  return pNode;
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

