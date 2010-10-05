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

#include <limits>
#include <math/Distance.h>
#include <ai/ml/clustering/Kmeans.h>

using namespace sai::ai;
using namespace sai::ai::ml;
using namespace sai::math;

Kmeans::Kmeans()
{
}

Kmeans::~Kmeans()
{
}

void 
Kmeans::activate()
{
  Partition::activate();

  const counter_t MAX       = _config->_maxIterations;
  const error_t   THRESHOLD = _config->_threshold; 
  error_t changed           = std::numeric_limits<error_t>::max();

  matrixsize_t isize = _inputs->size();
  matrixsize_t csize = _centers->size();
  matrixsize_t dsize = _inputs->front()->getRow();

  Distance * distance = 0;
  bool destroyDistanceNeeded = true;

  if (_config->_distance)
  {
    distance = _config->_distance;
    destroyDistanceNeeded = false;
  }
  else
  {
    distance = DistanceCreator().create(SAI_AI_DIST_EUCLIDEAN);
  }

  Vector dist (csize, 1);
  Matrix sum  (dsize, csize);
  Vector count(csize, 1);

  for (counter_t cnt = 0; cnt < MAX && changed > THRESHOLD; cnt += 1)
  {
    sum   = (matrixdata_t) 0.0;
    count = (matrixdata_t) 0.0;
    // 1. Calculate distance between points and centers
    for (matrixsize_t i = 0; i < isize; i += 1)
    {
      Vector * vector = _inputs->at(i);
      for (matrixsize_t c = 0; c < csize; c += 1)
      {
        Vector * center = _centers->at(c);

        matrixdata_t d = distance->dist2(*vector, *center);
        dist.set(c, 0, d);
      }

      matrixdata_t min;
      matrixsize_t ind;
      dist.min(&min, &ind);

      count.set(ind, 0, count.get(ind, 0) + 1);
      for (matrixsize_t d = 0; d < dsize; d += 1)
      {
        sum.set(d, ind, sum.get(d, ind) + vector->get(d, 0));
      }
    }

    // 2. Update centers
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      for (matrixsize_t d = 0; d < dsize; d += 1)
      {
        sum.set(d, c, sum.get(d, c) / count.get(c, 0));
      }
    }

    // 3. Calculate distance between old and new centers
    changed = (error_t) 0.0;
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      matrixdata_t d = distance->dist2(*_centers->at(c), 0, sum, c);
      changed += d;
    }

    // 4. Set new centers
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      Vector * center = _centers->at(c);
      for (matrixsize_t d = 0; d < dsize; d += 1)
      {
        center->set(d, 0, sum.get(d, c));
      }
    }
  }

  if (destroyDistanceNeeded)
  {
    delete distance;
  }
}

