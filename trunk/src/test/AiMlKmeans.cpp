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

#include <iostream>
#include <math/Types.h>
#include <math/Vector.h>
#include <ai/ml/clustering/Kmeans.h>

using namespace sai::math;
using namespace sai::ai;
using namespace sai::ai::ml;

void test1dim2centers22samples();
void print(VectorList& input, VectorList& center);
void cleanup(VectorList& input, VectorList& center);

int main(int argc, char * argv[])
{
  test1dim2centers22samples();
  return 0;
}

#define INIT_VECTOR(list,dim,data,entries) { \
          for(int _r = 0; _r < entries; _r += 1) { \
            Vector * v = new Vector(dim, 1); \
            for (int _d = 0; _d < dim; _d += 1) { \
              v->set(_d, 0, data[_r][_d]); } \
            list.push_back(v); }}

void 
test1dim2centers22samples()
{
  matrixdata_t cdata [][1] = {{1},
                              {2},
                              {3},
                              {3},
                              {4},
                              {5},
                              {5},
                              {3},
                              {3},
                              {5},
                              {8},
                              {5},
                              {9},
                              {5},
                              {7},
                              {5},
                              {8},
                              {8},
                              {8},
                              {8},
                              {7},
                              {7}};
  VectorList list;
  VectorList center;
  INIT_VECTOR(list, 1, cdata, 22);

  Kmeans kmeans;
  Configuration *config = kmeans.getConfiguration();
  config->set(KmeansConfiguration::MAX_ITERATION, (sai::ai::counter_t)50);
  config->set(KmeansConfiguration::THRESHOLD, (sai::ai::error_t)0.0001);
  config->set(KmeansConfiguration::NUM_CENTER, (sai::ai::counter_t)2);

  kmeans.setInput(&list);
  kmeans.setCenter(&center, 1);
  kmeans.activate();

  print(list, center);
  cleanup(list, center);
}

void 
print(VectorList& input, VectorList& center)
{
  std::cout << "++++++ CENTER ++++++" << std::endl;
  VectorListIterator iter;
  counter_t c = 0;
  for (iter  = center.begin();
       iter != center.end();
       iter ++, c++)
  {
    Vector * vector = *iter;
    std::cout << "Center[" << c << "]" << std::endl;
    vector->print();
  }
}

void 
cleanup(VectorList& input, VectorList& center)
{
  while (input.size() > 0)
  {
    Vector * v = input.front();
    input.erase(input.begin());
    delete v;
  }

  while (center.size() > 0)
  {
    Vector * v = center.front();
    center.erase(center.begin());
    delete v;
  }
}
