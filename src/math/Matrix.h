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

#ifndef __SAI_MATH_MATRIX__
#define __SAI_MATH_MATRIX__

#include <iostream>
#include <math/Types.h>

namespace sai
{
namespace math
{

class EigenSolver;
class Matrix
{
  friend class Vector;
  private:
    EigenSolver   *_eigenSolver;

  protected:
    matrixsize_t   _row;
    matrixsize_t   _col;
    matrixdata_t **_data;

  protected:
    void clean();

  public:
    Matrix();
    Matrix(matrixsize_t row, matrixsize_t col);
    virtual ~Matrix();

    virtual Matrix& operator= (const Matrix& rhs);
    virtual Matrix& operator+=(const Matrix& rhs);
    virtual Matrix& operator-=(const Matrix& rhs);

    virtual Matrix& operator= (matrixdata_t rhs);
    virtual Matrix& operator+=(matrixdata_t rhs);
    virtual Matrix& operator-=(matrixdata_t rhs);
    virtual Matrix& operator*=(matrixdata_t rhs);

    virtual void transpose(const Matrix& other);
    virtual void multiply(const Matrix& other, Matrix& result) const;
    virtual void inverse(Matrix& other);

    virtual void         init(matrixsize_t row, matrixsize_t col);
    virtual void         set(matrixsize_t row, matrixsize_t col, matrixdata_t data);
    virtual matrixdata_t get(matrixsize_t row, matrixsize_t col);
    virtual void         print(std::ostream& os = std::cout);

    EigenSolver* getEigenSolver();

    inline matrixsize_t getRow();
    inline matrixsize_t getCol();
};

inline matrixsize_t Matrix::getRow() { return _row; }
inline matrixsize_t Matrix::getCol() { return _col; }

}
}

#endif
