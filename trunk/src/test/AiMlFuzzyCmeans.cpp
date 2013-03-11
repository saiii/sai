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
#include <math/Utils.h>
#include <math/Vector.h>
#include <ai/ml/clustering/FuzzyCmeans.h>

using namespace sai::math;
using namespace sai::ai;
using namespace sai::ai::ml;

void test1dim2centers22samples();
void test2dim2centers22samples();
void print(VectorList& input, VectorList& center);
void cleanup(VectorList& input, VectorList& center);

int main(int argc, char * argv[])
{
  Utils::RandomSeed(10);
  test1dim2centers22samples();
  test2dim2centers22samples();
  return 0;
}

#define INIT_VECTOR(list,dim,data,entries) { \
          for(int _r = 0; _r < entries; _r += 1) { \
            Vector * v = new Vector(dim, 1); \
            for (int _d = 0; _d < dim; _d += 1) { \
              v->set(_d, 0, data[_r][_d]); } \
            list.push_back(v); }}

void 
test2dim2centers22samples()
{
  matrixdata_t cdata [][2] = {{1, 1},
                              {2, 1},
                              {3, 1},
                              {3, 1},
                              {4, 1},
                              {5, 1},
                              {5, 1},
                              {3, 1},
                              {3, 1},
                              {5, 1},
                              {8, 1},
                              {5, 1},
                              {9, 1},
                              {5, 1},
                              {7, 1},
                              {5, 1},
                              {8, 1},
                              {8, 1},
                              {8, 1},
                              {8, 1},
                              {7, 1},
                              {7, 1}};
  VectorList list;
  VectorList center;
  INIT_VECTOR(list, 2, cdata, 22);

  FuzzyCmeans fcm;
  Configuration *config = fcm.getConfiguration();
  config->set(PartitionConfiguration::MAX_ITERATION, (sai::ai::counter_t)50);
  config->set(PartitionConfiguration::THRESHOLD, (sai::ai::error_t)0.0001);
  config->set(PartitionConfiguration::NUM_CENTER, (sai::ai::counter_t)2);

  fcm.setInput(&list);
  fcm.setCenter(&center, 2);
  fcm.activate();
  print(list, center);
  cleanup(list, center);

  FuzzyCmeans fcm2;
  config = fcm2.getConfiguration();
  config->set(PartitionConfiguration::MAX_ITERATION, (sai::ai::counter_t)50);
  config->set(PartitionConfiguration::THRESHOLD, (sai::ai::error_t)0.0001);
  config->set(PartitionConfiguration::NUM_CENTER, (sai::ai::counter_t)2);
  config->set(PartitionConfiguration::RANDOM_CENTER, (sai::ai::counter_t)1);
  INIT_VECTOR(list, 2, cdata, 22);
  fcm2.setInput(&list);
  fcm2.setCenter(&center, 2);
  fcm2.activate();
  print(list, center);
  cleanup(list, center);

  FuzzyCmeans fcm3;
  config = fcm3.getConfiguration();
  config->set(PartitionConfiguration::MAX_ITERATION, (sai::ai::counter_t)50);
  config->set(PartitionConfiguration::THRESHOLD, (sai::ai::error_t)0.0001);
  config->set(PartitionConfiguration::NUM_CENTER, (sai::ai::counter_t)2);
  config->set(PartitionConfiguration::USER_SPECIFIED_CENTER, (sai::ai::counter_t)1);
  INIT_VECTOR(list, 2, cdata, 22);
  fcm3.setInput(&list);
  fcm3.setCenter(&center, 2);
  counter_t cntIndex [] = {10, 12};
  for (counter_t c = 0; c < 2; c += 1)
  {
    for (counter_t i = 0; i < 2; i += 1)
    {
      center.at(c)->set(i, 0, list.at(cntIndex[c])->get(i, 0));
    }
  }
  fcm3.activate();
  print(list, center);
  cleanup(list, center);
}

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

  FuzzyCmeans fcm;
  Configuration *config = fcm.getConfiguration();
  config->set(PartitionConfiguration::MAX_ITERATION, (sai::ai::counter_t)50);
  config->set(PartitionConfiguration::THRESHOLD, (sai::ai::error_t)0.0001);
  config->set(PartitionConfiguration::NUM_CENTER, (sai::ai::counter_t)2);

  fcm.setInput(&list);
  fcm.setCenter(&center, 1);
  fcm.activate();

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
