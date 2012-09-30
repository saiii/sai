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

#ifndef __SAI_AI_PARTITION__
#define __SAI_AI_PARTITION__

#include <math/Types.h>
#include <math/Distance.h>
#include <ai/Types.h>
#include <ai/ml/Configuration.h>

namespace sai { namespace ai { 
namespace ml 
{

class PartitionConfiguration;
class Partition
{
  protected:
    PartitionConfiguration * _config;
    sai::math::VectorList  * _inputs;
    sai::math::VectorList  * _centers;

  protected:
    Partition(bool);

  public:
    Partition();
    virtual ~Partition();

    virtual void           setInput(sai::math::VectorList* list); 
    virtual void           setCenter(sai::math::VectorList* list); 
    virtual void           setCenter(sai::math::VectorList* list, sai::math::matrixsize_t dim);
    virtual void           initCenter();
    virtual Configuration* getConfiguration(); 
    virtual void           activate() = 0; 
};

class PartitionConfiguration : public Configuration
{
  friend class Partition;
  friend class Kmeans;
  friend class FuzzyCmeans;
  protected:
    sai::ai::counter_t _maxIterations;
    sai::ai::counter_t _numCenter;
    sai::ai::error_t   _threshold;
    bool               _randomCenter;
    bool                  _userSpecifiedCenter;
    sai::math::Distance * _distance;

  public:
    typedef enum
    {
      MAX_ITERATION           = 1,
      NUM_CENTER              = 2,
      RANDOM_CENTER           = 3,
      USER_SPECIFIED_CENTER   = 4,
      THRESHOLD               = 5,
      USER_SPECIFIED_DISTANCE = 6
    }Type;

  public:
    PartitionConfiguration();
    virtual ~PartitionConfiguration();
    virtual void set(int, counter_t);
    virtual void set(int, error_t);
    virtual void set(int, void*);
};

}
}}

#endif
