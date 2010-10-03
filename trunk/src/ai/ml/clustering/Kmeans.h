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

#ifndef __SAI_AI_KMEANS__
#define __SAI_AI_KMEANS__

#include <math/Types.h>
#include <ai/Types.h>
#include <ai/ml/Configuration.h>

namespace sai { namespace ai { 
namespace ml 
{

class KmeansConfiguration;
class Kmeans
{
  private:
    KmeansConfiguration   * _config;
    sai::math::VectorList * _inputs;
    sai::math::VectorList * _centers;

  public:
    Kmeans();
    ~Kmeans();

    void                 setInput(sai::math::VectorList* list); 
    void                 setCenter(sai::math::VectorList* list); 
    void                 setCenter(sai::math::VectorList* list, sai::math::matrixsize_t dim);
    void                 initCenter();
    KmeansConfiguration* getConfiguration(); 
    void                 activate(); 
};

class KmeansConfiguration : public Configuration
{
  friend class Kmeans;
  private:
    sai::ai::counter_t _maxIterations;
    sai::ai::counter_t _numCenter;
    sai::ai::error_t   _threshold;
    bool               _randomCenter;

  public:
    typedef enum
    {
      MAX_ITERATION,
      NUM_CENTER,
      RANDOM_CENTER,
      THRESHOLD
    }Type;

  public:
    KmeansConfiguration();
    ~KmeansConfiguration();
    void set(int) {}
    void set(int, counter_t);
    void set(int, error_t);
};

}
}}

#endif
