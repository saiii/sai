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

#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include <math/Utils.h>

#include <iostream>

using namespace sai::math;

matrixsize_t 
Utils::RandomInt(matrixsize_t lower, matrixsize_t upper)
{
  return (lower + (matrixsize_t)(rand() % (upper - lower + 1)));
}

void 
Utils::RandomSeed()
{
  srand((uint32_t)time(0));
}

void 
Utils::RandomSeed(matrixsize_t s)
{
  srand(s);
}

