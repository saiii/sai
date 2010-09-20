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
#include <math/Matrix.h>
#include <math/EigenSolver.h>
#include <math/Vector.h>

using namespace sai::math;

#define INIT_MATRIX(matrix,rows,cols,data) { \
    for(int _r = 0; _r < rows; _r += 1) { \
      for(int _c = 0; _c < cols; _c += 1) { \
        matrix.set(_r,_c,data[_r][_c]); }}}

void eig22();
void eig33();
void displayResult(VectorList& vectors, MatrixDataList& values);

int main(int argc, char * argv[])
{
  eig22();
  eig33();
  return 0;
}

void 
displayResult(VectorList& vectors, MatrixDataList& values)
{
  VectorListIterator vecIter;
  MatrixDataListIterator valIter;

  for (vecIter  = vectors.begin();
       vecIter != vectors.end();
       vecIter ++)
  {
    Vector * vector = *vecIter;
    std::cout << "................." << std::endl;
    vector->print(std::cout);
  }

  std::cout << "+++++++++++++++++++++++++++" << std::endl;

  for (valIter  = values.begin();
       valIter != values.end();
       valIter ++)
  {
    std::cout << *valIter << std::endl;
    std::cout << "................." << std::endl;
  }
}

void 
eig33()
{
  matrixdata_t dat33 [][3] = {{1.0, 2.0, 3.0},
                              {2.0, 1.0, 1.0},
                              {4.0, 1.0, 1.0}};

  Matrix m3(3, 3);
  INIT_MATRIX(m3, 3, 3, dat33);
  m3.print();

  EigenSolver * solver = m3.getEigenSolver();
  solver->configure();
  solver->solve();

  VectorList vectors = solver->getEigenVectors();
  MatrixDataList values = solver->getEigenValues();

  displayResult(vectors, values);
}

void 
eig22()
{
  matrixdata_t dat22 [][2] = {{2.0, -12.0},
                              {1.0, - 5.0}};

  Matrix m2(2, 2);
  INIT_MATRIX(m2, 2, 2, dat22);
  m2.print();

  EigenSolver * solver = m2.getEigenSolver();
  solver->configure();
  solver->solve();

  VectorList vectors = solver->getEigenVectors();
  MatrixDataList values = solver->getEigenValues();

  displayResult(vectors, values);
}
