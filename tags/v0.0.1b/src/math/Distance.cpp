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

#include <math/Types.h>
#include <math/Distance.h>

using namespace sai::math;

class EuclideanDistance : public Distance
{
  public:
    inline matrixdata_t dist (Vector& a, Vector& b);
    inline matrixdata_t dist2(Vector& a, Vector& b);
    inline matrixdata_t dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
    inline matrixdata_t dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
};

class MahalanobisDistance : public Distance
{
  public:
    inline matrixdata_t dist (Vector& a, Vector& b);
    inline matrixdata_t dist2(Vector& a, Vector& b);
    inline matrixdata_t dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
    inline matrixdata_t dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
};

class ManhattanDistance : public Distance
{
  public:
    inline matrixdata_t dist (Vector& a, Vector& b);
    inline matrixdata_t dist2(Vector& a, Vector& b);
    inline matrixdata_t dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
    inline matrixdata_t dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
};

class MaximumNormDistance : public Distance
{
  public:
    inline matrixdata_t dist (Vector& a, Vector& b);
    inline matrixdata_t dist2(Vector& a, Vector& b);
    inline matrixdata_t dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
    inline matrixdata_t dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
};

class HammingDistance : public Distance
{
  public:
    inline matrixdata_t dist (Vector& a, Vector& b);
    inline matrixdata_t dist2(Vector& a, Vector& b);
    inline matrixdata_t dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
    inline matrixdata_t dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j);
};

Distance * 
DistanceCreator::create(DistanceType type)
{
  switch (type)
  {
    case SAI_AI_DIST_EUCLIDEAN:
      return new EuclideanDistance();
    case SAI_AI_DIST_MAHALANOBIS:
      return new MahalanobisDistance();
    case SAI_AI_DIST_MANHATTAN:
      return new ManhattanDistance();
    case SAI_AI_DIST_MAXIMUMNORM:
      return new MaximumNormDistance();
    case SAI_AI_DIST_HAMMING:
      return new HammingDistance();
    default:
      return 0;
  }
}

inline matrixdata_t
EuclideanDistance::dist(Vector& a, Vector& b)
{
  return sqrt(dist2(a, b));
}

inline matrixdata_t
EuclideanDistance::dist2(Vector& a, Vector& b)
{
  matrixdata_t val = (matrixdata_t) 0.0;
  matrixsize_t size = a.getRow();
  if (size > 1)
  {
    for (matrixsize_t r = 0; r < size; r += 1)
    {
      matrixdata_t diff = a.get(r,0) - b.get(r,0);
      val += (diff * diff);
    }
  }
  else
  {
    size = a.getCol();
    for (matrixsize_t c = 0; c < size; c += 1)
    {
      matrixdata_t diff = a.get(0,c) - b.get(0,c);
      val += (diff * diff);
    }
  }
  return val;
}

inline matrixdata_t 
EuclideanDistance::dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  return sqrt(dist2(a, i, b, j));
}

inline matrixdata_t 
EuclideanDistance::dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  matrixdata_t val = (matrixdata_t) 0.0;
  matrixsize_t size = a.getRow();

  for (matrixsize_t r = 0; r < size; r += 1)
  {
    matrixdata_t diff = a.get(r,i) - b.get(r,j);
    val += (diff * diff);
  }

  return val;
}

inline matrixdata_t
MahalanobisDistance::dist(Vector& a, Vector& b)
{
  return sqrt(dist2(a, b));
}

inline matrixdata_t
MahalanobisDistance::dist2(Vector& a, Vector& b)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t 
MahalanobisDistance::dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  return sqrt(dist2(a, i, b, j));
}

inline matrixdata_t 
MahalanobisDistance::dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t
ManhattanDistance::dist(Vector& a, Vector& b)
{
  return sqrt(dist2(a, b));
}

inline matrixdata_t
ManhattanDistance::dist2(Vector& a, Vector& b)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t 
ManhattanDistance::dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  return sqrt(dist2(a, i, b, j));
}

inline matrixdata_t 
ManhattanDistance::dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t
MaximumNormDistance::dist(Vector& a, Vector& b)
{
  return sqrt(dist2(a, b));
}

inline matrixdata_t
MaximumNormDistance::dist2(Vector& a, Vector& b)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t 
MaximumNormDistance::dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  return sqrt(dist2(a, i, b, j));
}

inline matrixdata_t 
MaximumNormDistance::dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t
HammingDistance::dist(Vector& a, Vector& b)
{
  return sqrt(dist2(a, b));
}

inline matrixdata_t
HammingDistance::dist2(Vector& a, Vector& b)
{
  // TODO
  return (matrixdata_t) 0.0;
}

inline matrixdata_t 
HammingDistance::dist (Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  return sqrt(dist2(a, i, b, j));
}

inline matrixdata_t 
HammingDistance::dist2(Matrix& a, matrixsize_t i, Matrix& b, matrixsize_t j)
{
  // TODO
  return (matrixdata_t) 0.0;
}

