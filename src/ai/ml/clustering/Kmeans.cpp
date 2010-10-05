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
#include <stdexcept>
#include <math/Distance.h>
#include <math/Utils.h>
#include <ai/ml/clustering/Kmeans.h>

using namespace sai::ai;
using namespace sai::ai::ml;
using namespace sai::math;

Kmeans::Kmeans():
  _config(0),
  _inputs(0),
  _centers(0)
{
  _config = new KmeansConfiguration();
}

Kmeans::~Kmeans()
{
  delete _config;
}

void 
Kmeans::setInput(VectorList* list)
{
  if (!(list->front()->getRow() >= 1 && list->front()->getCol() == 1))
  {
    throw std::invalid_argument("Input must be column vector");
  }
  _inputs = list;
}

void 
Kmeans::setCenter(VectorList* list) 
{
  setCenter(list, 0);
}

void 
Kmeans::setCenter(sai::math::VectorList* list, matrixsize_t dim)
{
  if (list->size() > _config->_numCenter)
  {
    counter_t rem = list->size() - _config->_numCenter;
    for (counter_t i = 0; i < rem; i += 1)
    {
      Vector * v = list->front();
      list->erase(list->begin());
      delete v;
    }
    _centers = list;
    return;
  }

  matrixsize_t row = dim;

  if (dim == 0)
  {
    if (list->size() == 0)
    {
      throw std::invalid_argument("To use this function, the given list of "
        "center must contain at least one center");
    }

    if (!(list->front()->getRow() >= 1 && list->front()->getCol() == 1))
    {
      throw std::invalid_argument("Centers must be column vectors");
    }
    row = list->front()->getRow();
  }

  for (counter_t i = list->size(); i < _config->_numCenter; i += 1)
  {
    Vector * c = new Vector(row, 1);
    list->push_back(c);
  }

  _centers = list;
}

void 
Kmeans::initCenter()
{
  if (_inputs->size() == 0)
  {
    throw std::invalid_argument("This function requires input list");
  }

  if (_centers->size() == 0)
  {
    throw std::invalid_argument("This function requires center list");
  }

  if (_centers->size() > _inputs->size())
  {
    throw std::invalid_argument("Number of element in center list must "
      "less than the number of element in input list");
  }

  counter_t dim = _inputs->front()->getRow();
  if (_config->_randomCenter)
  {
    Vector v(_centers->size(), 1);
    v = 0.0;
    for (counter_t i = 0; i < _centers->size(); i += 1)
    {
      counter_t r = Utils::RandomInt(0, _centers->size() - 1);
      if (v.get(i, 0) == 0.0)
      {
        v.set(i, 0, (matrixdata_t)r);
      }
      else
      {
        i -= 1;
      }
    }

    for (counter_t i = 0; i < _centers->size(); i += 1)
    {
      for (counter_t d = 0; d < dim; d += 1)
      {
        _centers->at(i)->set(d, 0, _inputs->at(v.get(i, 0))->get(d,0));
      }
    }
  }
  else
  {
    for (counter_t i = 0; i < _centers->size(); i += 1)
    {
      for (counter_t d = 0; d < dim; d += 1)
      {
        _centers->at(i)->set(d, 0, _inputs->at(i)->get(d, 0));
      }
    }
  }
}

KmeansConfiguration* 
Kmeans::getConfiguration() 
{
  return _config;
}

void 
Kmeans::activate()
{
  initCenter();
  const counter_t MAX       = _config->_maxIterations;
  const error_t   THRESHOLD = _config->_threshold; 
  error_t changed           = std::numeric_limits<error_t>::max();

  matrixsize_t isize = _inputs->size();
  matrixsize_t csize = _centers->size();
  matrixsize_t dsize = _inputs->front()->getRow();

  Distance * distance = DistanceCreator().create(SAI_AI_DIST_EUCLIDEAN);
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

  delete distance;
}

//-----------------------------------------------------------------------------

KmeansConfiguration::KmeansConfiguration():
  _maxIterations(1000),
  _numCenter(1),
  _threshold(0.000001),
  _randomCenter(false)
{}

KmeansConfiguration::~KmeansConfiguration()
{}

void 
KmeansConfiguration::set(int t, counter_t c)
{
  switch(t)
  {
    case MAX_ITERATION:
      _maxIterations = c;
      break;
    case NUM_CENTER:
      _numCenter = c;
      break;
    case RANDOM_CENTER:
      _randomCenter = c == 0 ? false : true;
      break;
    case THRESHOLD:
      break;
  }
}

void 
KmeansConfiguration::set(int t, error_t e)
{
  switch(t)
  {
    case MAX_ITERATION:
      break;
    case NUM_CENTER:
      break;
    case RANDOM_CENTER:
      break;
    case THRESHOLD:
      _threshold = e;
      break;
  }
}
