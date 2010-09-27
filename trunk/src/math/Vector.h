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

#ifndef __SAI_MATH_VECTOR__
#define __SAI_MATH_VECTOR__

#include <cmath>
#include <math/Matrix.h>

namespace sai
{
namespace math
{

class Vector : public Matrix
{
  private:
    typedef enum 
    {
      SAI_MATH_ROW_VECTOR,
      SAI_MATH_COL_VECTOR
    }Type;

    Type _type;

  public:
    Vector();
    Vector(matrixsize_t row, matrixsize_t col);
    ~Vector();

    Matrix& operator= (const Matrix& rhs);
    Matrix& operator+=(const Matrix& rhs);
    Matrix& operator-=(const Matrix& rhs);

    Matrix& operator= (matrixdata_t rhs);
    Matrix& operator+=(matrixdata_t rhs);
    Matrix& operator-=(matrixdata_t rhs);
    Matrix& operator*=(matrixdata_t rhs);

    void init(matrixsize_t row, matrixsize_t col);

    void         max(matrixdata_t *mx, matrixsize_t *index = 0);
    void         min(matrixdata_t *mn, matrixsize_t *index = 0);
    matrixdata_t sum();
};

}
}

#endif
