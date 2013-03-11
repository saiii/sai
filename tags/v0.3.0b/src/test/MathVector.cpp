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

#include <math/Vector.h>
#include <math/Distance.h>

using namespace sai::math;

#define ARGUMENTS_1 Vector *&a, Vector *&b, Vector *&c, Vector *&d
#define ARGUMENTS_2 Vector &a, Vector &b, Vector &c, Vector &d
#define ARGUMENTS_3 a,b,c,d
#define ARGUMENTS_4 *a,*b,*c,*d

#define INIT_VECTOR(matrix,dim,data) { \
    for(int _d = 0; _d < dim; _d += 1) { \
      matrix.set(_d, 0, data[_d]); }}


void constructor(ARGUMENTS_1);
void destructor(ARGUMENTS_1);
void set(ARGUMENTS_2);
void print(ARGUMENTS_2);
void assign(ARGUMENTS_2);
void additionAssignment(ARGUMENTS_2);
void subtractionAssignment(ARGUMENTS_2);
void multiplicationAssignment(ARGUMENTS_2);
void transpose(ARGUMENTS_2);
void get(ARGUMENTS_2);
void max(ARGUMENTS_2);
void sum(ARGUMENTS_2);
void dist(ARGUMENTS_2);

int main(int argc, char * argv[])
{
  Vector *a, *b, *c, *d;
  constructor(ARGUMENTS_3);

  set(ARGUMENTS_4);
  print(ARGUMENTS_4);
  assign(ARGUMENTS_4);
  additionAssignment(ARGUMENTS_4);
  subtractionAssignment(ARGUMENTS_4);
  multiplicationAssignment(ARGUMENTS_4);
  transpose(ARGUMENTS_4);
  get(ARGUMENTS_4);
  max(ARGUMENTS_4);
  sum(ARGUMENTS_4);
  dist(ARGUMENTS_4);

  destructor(ARGUMENTS_3);
  return 0;
}

void 
dist(ARGUMENTS_2)
{
  std::cout << "Dist" << std::endl;
  matrixdata_t val;

  Distance * distance1 = DistanceCreator().create(SAI_AI_DIST_EUCLIDEAN);
  Distance * distance2 = DistanceCreator().create(SAI_AI_DIST_MAHALANOBIS);

  c.init(a.getRow(), a.getCol());
  c  = a;
  c += (matrixdata_t)2.0;

  std::cout << "A = "; a.print();
  std::cout << "C = "; c.print();
  val = distance1->dist2(a,c);
  std::cout << "Dist2 = " << val << std::endl;
  val = distance2->dist2(a,c);
  std::cout << "Dist2 = " << val << std::endl;
  val = distance1->dist(a,c);
  std::cout << "Dist = " << val << std::endl;
  val = distance2->dist(a,c);
  std::cout << "Dist = " << val << std::endl;
  
  c.init(b.getRow(), b.getCol());
  c  = b;
  c += (matrixdata_t)2.0;

  std::cout << "B = "; b.print();
  std::cout << "C = "; c.print();
  val = distance1->dist2(b,c);
  std::cout << "Dist2 = " << val << std::endl;
  val = distance2->dist2(b,c);
  std::cout << "Dist2 = " << val << std::endl;
  val = distance1->dist(b,c);
  std::cout << "Dist = " << val << std::endl;
  val = distance2->dist(b,c);
  std::cout << "Dist = " << val << std::endl;

  delete distance2;
  delete distance1;
}

void 
sum(ARGUMENTS_2)
{
  std::cout << "Sum" << std::endl;
  matrixdata_t val;

  a.print();
  val = a.sum();
  std::cout << "Sum = " << val << std::endl;

  b.print();
  val = b.sum();
  std::cout << "Sum = " << val << std::endl;
}

void 
max(ARGUMENTS_2)
{
  std::cout << "Max" << std::endl;
  matrixdata_t mval;
  matrixsize_t mind;

  a.print();
  a.max(&mval, &mind);
  std::cout << "Max = " << mval << ", Ind = " << mind << std::endl;

  b.print();
  b.max(&mval, &mind);
  std::cout << "Max = " << mval << ", Ind = " << mind << std::endl;
}

void 
get(ARGUMENTS_2)
{
  matrixsize_t row = d.getRow();

  std::cout << "Get" << std::endl;
  d.print();
  for (matrixsize_t r = 0; r < row; r += 1)
  {
    std::cout << d.get(r, 0) << " " << std::endl;
  }
}

void 
transpose(ARGUMENTS_2)
{
  std::cout << "Transpose" << std::endl;
  matrixdata_t adata [] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  
  a.init(6, 1);
  INIT_VECTOR(a, 6, adata);
  a.print();

  b.transpose(a);
  b.print();
}

void 
multiplicationAssignment(ARGUMENTS_2)
{
  std::cout << "Multiplication Assignment" << std::endl;
  a = b;
  a.print();
  
  a *= (matrixdata_t) 2.0;
  a.print();
}

void 
subtractionAssignment(ARGUMENTS_2)
{
  std::cout << "Substraction Assignment" << std::endl;
  a = (matrixdata_t)0.0;
  a.print();
  a -= b;
  a.print();
  a += b;
  a.print();

  a = b;
  a.print();
  a -= (matrixdata_t)1.0;
  a.print();
}

void 
additionAssignment(ARGUMENTS_2)
{
  std::cout << "Addition Assignment" << std::endl;
  a = (matrixdata_t)0.0;
  a.print();
  a += b;
  a.print();
  a += b;
  a.print();

  a += (matrixdata_t) 1.0;
  a.print();
}

void 
assign(ARGUMENTS_2)
{
  a.init(c.getRow(), 1);
  b.init(d.getRow(), 1);

  a = c;
  b = d;
  std::cout << "Assign" << std::endl;
  print(ARGUMENTS_3);
}

void 
print(ARGUMENTS_2)
{
  a.print();
  b.print();
  c.print();
  d.print();
}

void
set(ARGUMENTS_2)
{
  matrixdata_t cdata [5] = {1, 2, 3, 4, 5};
  INIT_VECTOR(c, 5, cdata);  

  matrixdata_t ddata [5] = {10, 11, 12, 13, 14};
  INIT_VECTOR(d, 5, ddata);
}

void
destructor(ARGUMENTS_1)
{
  delete d;
  delete c;
  delete b;
  delete a;
}

void 
constructor(ARGUMENTS_1)
{
  a = new Vector();
  b = new Vector();
  c = new Vector(5,1);
  d = new Vector(5,1);
}

