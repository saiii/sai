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

#include "BinarySearch.h"
#include "List.h"

using namespace sai::utils;

Element * 
BinarySearch::search(List* list, Key* key)
{
  index_t size = list->size();
  index_t l,m,r;
  l = 0;
  r = size - 1;
  if (list->at(0)->operator<(list->at(size-1))) // Ascending sorted list
  {
    do
    {
      m = l + ((r - l)/2);
      if (list->at(m)->operator>(key))
      {
        r = m - 1;
      }
      else
      {
        l = m + 1;
      }

      if (list->at(m)->operator==(key))
      {
        return list->at(m);
      }

      if (l == r)
      {
        if (list->at(l)->operator==(key))
        {
          return list->at(l);
        }
        return 0;
      }
    }while(l < r);
  }
  else // Descending sorted list
  {
    do
    {
      m = l + ((r - l)/2);
      if (list->at(m)->operator>(key))
      {
        l = m + 1;
      }
      else
      {
        r = m - 1;
      }

      if (list->at(m)->operator==(key))
      {
        return list->at(m);
      }

      if (l == r)
      {
        if (list->at(l)->operator==(key))
        {
          return list->at(l);
        }
        return 0;
      }
    }while(l < r);
  }
  return 0;
}

