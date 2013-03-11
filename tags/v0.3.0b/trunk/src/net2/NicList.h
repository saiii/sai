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

#ifndef __SAI_NET2_NICLIST__
#define __SAI_NET2_NICLIST__

#include <stdint.h>
#include <vector>
#include <string>

namespace sai
{
namespace net2
{
class Nic;
typedef std::vector<Nic*>           _NicList;
typedef std::vector<Nic*>::iterator _NicListIterator;        

class NicList
{
friend class Nic;
private:
  Nic        * _defaultNic;
  Nic        * _currentNic;
  _NicList     _nicList;
  std::string  _hostAddress;

private:
  void sortNicList(_NicList& list);

public:
  NicList();
  ~NicList();

  void  detect();
  Nic * getCurrentNic() { return _currentNic; }
  Nic * getDefaultNic(); 
  Nic * getNic(const char * ip);

  uint16_t size() { return (uint16_t)_nicList.size(); }
  Nic * getNic(uint16_t index) { return _nicList.at(index); } 
};

}}

#endif
