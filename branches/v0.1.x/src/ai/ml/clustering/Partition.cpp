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

#include <stdexcept>
#include <math/Distance.h>
#include <math/Utils.h>
#include <ai/ml/clustering/Partition.h>

using namespace sai::ai;
using namespace sai::ai::ml;
using namespace sai::math;

Partition::Partition(bool):
  _config(0),
  _inputs(0),
  _centers(0)
{
}

Partition::Partition():
  _config(0),
  _inputs(0),
  _centers(0)
{
  _config = new PartitionConfiguration();
}

Partition::~Partition()
{
  delete _config;
}

void 
Partition::setInput(VectorList* list)
{
  if (!(list->front()->getRow() >= 1 && list->front()->getCol() == 1))
  {
    throw std::invalid_argument("Input must be column vector");
  }
  _inputs = list;
}

void 
Partition::setCenter(VectorList* list) 
{
  setCenter(list, 0);
}

void 
Partition::setCenter(sai::math::VectorList* list, matrixsize_t dim)
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
Partition::initCenter()
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
    Vector v(_inputs->size(), 1);
    v = 0.0;
    for (counter_t i = 0; i < _centers->size(); i += 1)
    {
      counter_t r = Utils::RandomInt(0, _inputs->size() - 1);
      if (v.get(r, 0) == 0.0)
      {
        v.set(r, 0, (matrixdata_t)1.0);
      }
      else
      {
        i -= 1;
      }
    }

    counter_t c = 0;
    for (counter_t i = 0; i < _inputs->size(); i += 1)
    {
      if (v.get(i,0))
      {
        for (counter_t d = 0; d < dim; d += 1)
        {
          _centers->at(c)->set(d, 0, _inputs->at(i)->get(d,0));
        }
        c += 1;
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

Configuration* 
Partition::getConfiguration() 
{
  return _config;
}

void 
Partition::activate()
{
  if (!_config->_userSpecifiedCenter)
  {
    initCenter();
  }
}

//-----------------------------------------------------------------------------

PartitionConfiguration::PartitionConfiguration():
  _maxIterations(1000),
  _numCenter(1),
  _threshold(0.000001),
  _randomCenter(false),
  _userSpecifiedCenter(false),
  _distance(0)
{}

PartitionConfiguration::~PartitionConfiguration()
{}

void 
PartitionConfiguration::set(int t, counter_t c)
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
      _userSpecifiedCenter = _randomCenter ? false : _userSpecifiedCenter;
      break;
    case USER_SPECIFIED_CENTER:
      _userSpecifiedCenter = c == 0 ? false : true;
      _randomCenter = _userSpecifiedCenter ? false : _randomCenter;
      break;
    default:
      break;
  }
}

void 
PartitionConfiguration::set(int t, error_t e)
{
  switch(t)
  {
    case THRESHOLD:
      _threshold = e;
      break;
    default:
      break;
  }
}

void 
PartitionConfiguration::set(int t, void* v)
{
  switch (t)
  {
    case USER_SPECIFIED_DISTANCE:
      _distance = (sai::math::Distance*)(v);
      break;
    default:
      break;
  }
}
