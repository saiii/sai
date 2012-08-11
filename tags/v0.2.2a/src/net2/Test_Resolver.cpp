//=============================================================================
// Copyright (C) 2012 Supanut Panyagosa <toy.mastersonic@gmail.com>
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

#include <stdio.h>
#include <string>

#include <net2/Resolver.h>
#include <net2/Net.h>

using namespace sai::net2;
int main(int argc, char * argv[])
{
  Resolver *r = new Resolver();
  std::string ip;
  r->getIpFromName("www.google.com", ip);
  printf("%s\n", ip.c_str());
  return 0;
}
