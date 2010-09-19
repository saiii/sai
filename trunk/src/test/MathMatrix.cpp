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

#include <math/Matrix.h>

using namespace sai::math;

#define ARGUMENTS_1 Matrix *&a, Matrix *&b, Matrix *&c, Matrix *&d
#define ARGUMENTS_2 Matrix &a, Matrix &b, Matrix &c, Matrix &d
#define ARGUMENTS_3 a,b,c,d
#define ARGUMENTS_4 *a,*b,*c,*d

#define INIT_MATRIX(matrix,rows,cols,data) { \
    for(int _r = 0; _r < rows; _r += 1) { \
      for(int _c = 0; _c < cols; _c += 1) { \
        matrix.set(_r,_c,data[_r][_c]); }}}


void constructor(ARGUMENTS_1);
void destructor(ARGUMENTS_1);
void set(ARGUMENTS_2);
void print(ARGUMENTS_2);
void assign(ARGUMENTS_2);
void additionAssignment(ARGUMENTS_2);
void subtractionAssignment(ARGUMENTS_2);
void multiplicationAssignment(ARGUMENTS_2);
void transpose(ARGUMENTS_2);

int main(int argc, char * argv[])
{
  Matrix *a, *b, *c, *d;
  constructor(ARGUMENTS_3);

  set(ARGUMENTS_4);
  print(ARGUMENTS_4);
  assign(ARGUMENTS_4);
  additionAssignment(ARGUMENTS_4);
  subtractionAssignment(ARGUMENTS_4);
  multiplicationAssignment(ARGUMENTS_4);
  transpose(ARGUMENTS_4);

  destructor(ARGUMENTS_3);
  return 0;
}

void 
transpose(ARGUMENTS_2)
{
  std::cout << "Transpose" << std::endl;
  matrixdata_t adata [][5] = {{1.0}, {2.0}, {3.0}, {4.0}, {5.0}, {6.0}};
  
  a.init(6, 1);
  INIT_MATRIX(a, 6, 1, adata);
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
  a.init(c.getRow(), c.getCol());
  b.init(d.getRow(), d.getCol());

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
  matrixdata_t cdata [][5] = {{1, 2, 3, 4, 5}, 
                              {2, 3, 4, 5, 6}, 
                              {3, 4, 5, 6, 7}, 
                              {4, 5, 6, 7, 8}, 
                              {5, 6, 7, 8, 9}};
  INIT_MATRIX(c, 5, 5, cdata);  

  matrixdata_t ddata [][5] = {{10, 11, 12, 13, 14},
                              {11, 12, 13, 14, 15},
                              {12, 13, 14, 15, 16},
                              {13, 14, 15, 16, 17},
                              {14, 15, 16, 17, 18}};
  INIT_MATRIX(d, 5, 5, ddata);
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
  a = new Matrix();
  b = new Matrix();
  c = new Matrix(5, 5);
  d = new Matrix(5, 5);
}

