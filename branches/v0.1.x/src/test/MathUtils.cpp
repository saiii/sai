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

#include <cstdlib>
#include <iostream>
#include <math/Utils.h>

using namespace sai::math;

int main(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Invalid argument!" << std::endl;
    return 1;
  }

  Utils::RandomSeed();
  std::cout << "Random = " << Utils::RandomInt(atoi(argv[1]), atoi(argv[2])) << std::endl;
  return 0;
}
