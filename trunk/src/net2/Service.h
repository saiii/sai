//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
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

#ifndef __SAI_NET2_SERVICE__
#define __SAI_NET2_SERVICE__

#include <vector>

namespace sai
{
namespace net2
{

class Nic;
class Service
{
public:
  Service();
  virtual ~Service();

  virtual void activate() = 0;
  virtual void deactivate() = 0;
};

typedef std::vector<Service*>           ServiceList;
typedef std::vector<Service*>::iterator ServiceListIterator;

}}

#endif
