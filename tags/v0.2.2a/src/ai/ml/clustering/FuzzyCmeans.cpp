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

#include <cmath>
#include <limits>
#include <math/Distance.h>
#include <ai/ml/clustering/FuzzyCmeans.h>

using namespace sai::ai;
using namespace sai::ai::ml;
using namespace sai::math;

FuzzyCmeans::FuzzyCmeans():
  Partition(false)
{
  _config = new FuzzyCmeansConfiguration();
}

FuzzyCmeans::~FuzzyCmeans()
{
  delete _config;
  _config = 0;
}

void 
FuzzyCmeans::activate()
{
  Partition::activate();

  FuzzyCmeansConfiguration * config = dynamic_cast<FuzzyCmeansConfiguration*>(_config);

  const counter_t MAX       = config->_maxIterations;
  const error_t   THRESHOLD = config->_threshold; 
  error_t changed           = std::numeric_limits<error_t>::max();

  matrixsize_t isize = _inputs->size();
  matrixsize_t csize = _centers->size();
  matrixsize_t dsize = _inputs->front()->getRow();

  Distance * distance = 0;
  bool destroyDistanceNeeded = true;

  if (config->_distance)
  {
    distance = config->_distance;
    destroyDistanceNeeded = false;
  }
  else
  {
    distance = DistanceCreator().create(SAI_AI_DIST_EUCLIDEAN);
  }

  Matrix membership(csize, isize);
  Vector tcenter1  (dsize, 1);
  Vector tcenter2  (dsize, 1);

  matrixdata_t pfuzzifier = (matrixdata_t)(1.0 / ((double)config->_fuzzifier - 1.0));
  matrixdata_t fuzzifier  = config->_fuzzifier;

  Vector ** newcenter = new Vector*[csize];
  for (counter_t c = 0; c < csize; c += 1)
  {
    newcenter[c] = new Vector(dsize, 1);
  }

  for (counter_t cnt = 0; cnt < MAX && changed > THRESHOLD; cnt += 1)
  {
    // 1. Update membership
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      for (matrixsize_t i = 0; i < isize; i += 1)
      {
        matrixdata_t sum = 0.0;
        matrixdata_t d1  = distance->dist2(*_inputs->at(i), *_centers->at(c));

        for (matrixsize_t k = 0; k < csize; k += 1)
        {
          matrixdata_t d2 = distance->dist2(*_inputs->at(i), *_centers->at(k));
          if (d2 == 0.0) 
          {
            d2 = std::numeric_limits<matrixdata_t>::min();
          }
          matrixdata_t tmp = d1 / d2;
          sum += pow(tmp, pfuzzifier);
        }

        sum = (sum == 0.0) ? 1.0 : 1.0/sum;
        membership.set(c, i, pow(sum, fuzzifier));
      }
    }

    // 2. Update centers
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      matrixdata_t d = (matrixdata_t)0.0;
      tcenter1 = (matrixdata_t)0.0;
      for (matrixsize_t i = 0; i < isize; i += 1)
      {
        tcenter2  = *_inputs->at(i);
        d        += membership.get(c, i);
        tcenter2 *= membership.get(c, i);
        tcenter1 += tcenter2;
      }
      tcenter1 *= (matrixdata_t)(1.0/d);
      newcenter[c]->operator=(tcenter1);
    }

    // 3. Calculate distance between old and new centers
    changed = (error_t) 0.0;
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      matrixdata_t d = distance->dist2(*_centers->at(c), *newcenter[c]);
      changed += d;
    }

    // 4. Set new centers
    for (matrixsize_t c = 0; c < csize; c += 1)
    {
      _centers->at(c)->operator=(*newcenter[c]);
    }
  }

  for (counter_t c = 0; c < csize; c += 1)
  {
    delete newcenter[c];
  }
  delete [] newcenter;

  if (destroyDistanceNeeded)
  {
    delete distance;
  }
}

FuzzyCmeansConfiguration::FuzzyCmeansConfiguration():
  _fuzzifier(1.5)
{
}

FuzzyCmeansConfiguration::~FuzzyCmeansConfiguration()
{
}

void 
FuzzyCmeansConfiguration::set(int t, error_t e)
{
  switch (t)
  {
    case FUZZIFIER:
      _fuzzifier = (error_t)e;
      break;
    default:
      PartitionConfiguration::set(t,e);
      break;
  }
}

