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

#include <cstdlib>
#include <limits>
#include <math/Vector.h>

using namespace sai::math;

Vector::Vector(): 
  Matrix(),
  _type(SAI_MATH_ROW_VECTOR)
{
}

Vector::Vector(matrixsize_t row, matrixsize_t col):
  Matrix()
{
  if (col == 1)
  {
    _type = SAI_MATH_ROW_VECTOR;
    init(row, 1);
  }
  else
  {
    _type = SAI_MATH_COL_VECTOR;
    init(1, col);
  }
}

Vector::~Vector()
{
  clean();
}

Matrix& 
Vector::operator= (const Matrix& rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] = rhs._data[r][0];
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] = rhs._data[0][c];
    }
  }
  return *this;
}

Matrix& 
Vector::operator+=(const Matrix& rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] += rhs._data[r][0];
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] += rhs._data[0][c];
    }
  }
  return *this;
}

Matrix& 
Vector::operator-=(const Matrix& rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] -= rhs._data[r][0];
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] -= rhs._data[0][c];
    }
  }
  return *this;
}

Matrix& 
Vector::operator= (matrixdata_t rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] = rhs;
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] = rhs;
    }
  }
  return *this;
}

Matrix& 
Vector::operator+=(matrixdata_t rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] += rhs;
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] += rhs;
    }
  }
  return *this;
}

Matrix& 
Vector::operator-=(matrixdata_t rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] -= rhs;
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] -= rhs;
    }
  }
  return *this;
}

Matrix& 
Vector::operator*=(matrixdata_t rhs)
{
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      _data[r][0] *= rhs;
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[0][c] *= rhs;
    }
  }
  return *this;
}

void
Vector::init(matrixsize_t row, matrixsize_t col)
{

  if (row == _row && col == _col) return;

  clean();
  if (col == 1)
  {
    _type = SAI_MATH_ROW_VECTOR;
    _row = row;
    _col = 1;

    _data = new matrixdata_t*[_row];
    for (matrixsize_t i = 0; i < _row; i += 1)
    {
      _data[i] = new matrixdata_t[1];
    }
  }
  else
  {
    _type = SAI_MATH_COL_VECTOR;
    _row = 1;
    _col = col;

    _data = new matrixdata_t*[1];
    _data[0] = new matrixdata_t[_col];
  }
}

void
Vector::max(matrixdata_t *mx, matrixsize_t *index)
{
  matrixdata_t val = (matrixdata_t) 0.0;
  matrixsize_t ind = std::numeric_limits<matrixsize_t>::min();
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      if (fabs(_data[r][0]) > fabs(val))
      {
        val = _data[r][0];
        ind = r;
      }
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      if (fabs(_data[0][c]) > fabs(val))
      {
        val = _data[0][c];
        ind = c;
      }
    }
  }
  
  if (mx)
  {
    *mx = val;
  }

  if (index)
  {
    *index = ind;
  }
}

void
Vector::min(matrixdata_t *mn, matrixsize_t *index)
{
  matrixdata_t val = _data[0][0];
  matrixsize_t ind = std::numeric_limits<matrixsize_t>::min();
  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      if (fabs(_data[r][0]) < fabs(val))
      {
        val = _data[r][0];
        ind = r;
      }
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      if (fabs(_data[0][c]) < fabs(val))
      {
        val = _data[0][c];
        ind = c;
      }
    }
  }
  
  if (mn)
  {
    *mn = val;
  }

  if (index)
  {
    *index = ind;
  }
}

matrixdata_t 
Vector::sum()
{
  matrixdata_t val = (matrixdata_t) 0.0;

  if (_type == SAI_MATH_ROW_VECTOR)
  {
    for (matrixsize_t r = 0; r < _row; r += 1)
    {
      val += _data[r][0];
    }
  }
  else
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      val += _data[0][c];
    }
  }

  return val;
}

