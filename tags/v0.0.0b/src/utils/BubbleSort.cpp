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

#include "BubbleSort.h"
#include "List.h"

using namespace sai::utils;

BubbleSort::BubbleSort()
{}

BubbleSort::~BubbleSort()
{}

void 
BubbleSort::sort(List * list, Comparer* comp)
{
  index_t x,i,j;
  x = 1;

  while (x < list->size())
  {
    i  = x - 1;
    j  = x;
    x += 1;
    Element * ei = list->at(i);
    Element * ej = list->at(j);
    if (comp->compare(ei, ej))
    {
      list->swap(i, j);
      x = 1;
    }
  }
}

