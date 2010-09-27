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

#ifndef __SAI_MATH_DISTANCE__
#define __SAI_MATH_DISTANCE__

#include <math/Vector.h>

namespace sai 
{
namespace math
{

class Distance
{
  public:
    virtual matrixdata_t dist (sai::math::Vector& a, sai::math::Vector& b) = 0;
    virtual matrixdata_t dist2(sai::math::Vector& a, sai::math::Vector& b) = 0;
};

typedef enum
{
  SAI_AI_DIST_EUCLIDEAN,
  SAI_AI_DIST_MAHALANOBIS
}DistanceType;

class DistanceCreator
{
  public:
    Distance * create(DistanceType);
};

}
}
#endif
