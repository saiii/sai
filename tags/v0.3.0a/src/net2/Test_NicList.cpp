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

#include <iostream>
#include <net2/Net.h>
#include <net2/Nic.h>
#include <net2/NicList.h>

using namespace sai::net2;

int main(int argc, char * argv[])
{
  NicList * nList = new NicList();
  nList->detect();

  std::cout << "Size = " << nList->size() << std::endl;
  for (int i = 0; i < nList->size(); i += 1)
  {
    std::string nic;
    nList->getNic(i)->toString(nic);
    std::cout << nic << std::endl;
  }
  delete nList;
  return 0;
}
