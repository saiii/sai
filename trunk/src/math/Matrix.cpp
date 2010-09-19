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

#include <string.h>
#include <iomanip>

#include <math/Matrix.h>

using namespace sai::math;

Matrix::Matrix():
  _row((matrixsize_t)0),
  _col((matrixsize_t)0),
  _data(0)
{
}

Matrix::Matrix(matrixsize_t row, matrixsize_t col):
  _row(0),
  _col(0),
  _data(0)
{
  init(row, col);
}

Matrix::~Matrix()
{
  clean();
}

void 
Matrix::clean()
{
  for (matrixsize_t i = 0; i < _row; i += 1)
  {
    delete [] _data[i];
  }
  delete [] _data;
}

Matrix& 
Matrix::operator= (const Matrix& rhs)
{
  unsigned long sz = sizeof(_data[0][0]) * _col;
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    memcpy(_data[r], rhs._data[r], sz);
  }
  return *this;
}

Matrix& 
Matrix::operator+=(const Matrix& rhs)
{
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] += rhs._data[r][c];
    }
  }
  return *this;
}

Matrix& 
Matrix::operator-=(const Matrix& rhs)
{
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] -= rhs._data[r][c];
    }
  }
  return *this;
}

Matrix& 
Matrix::operator= (matrixdata_t rhs)
{
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] = rhs;
    }
  }
  return *this;
}

Matrix& 
Matrix::operator+=(matrixdata_t rhs)
{
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] += rhs;
    }
  }
  return *this;
}

Matrix& 
Matrix::operator-=(matrixdata_t rhs)
{
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] -= rhs;
    }
  }
  return *this;
}

Matrix& 
Matrix::operator*=(matrixdata_t rhs)
{
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] *= rhs;
    }
  }
  return *this;
}

void 
Matrix::transpose(const Matrix& other)
{
  init(other._col, other._row);
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      _data[r][c] = other._data[c][r];
    }
  }
}

void 
Matrix::multiply(const Matrix& other, Matrix& result) const
{
}

void 
Matrix::inverse(Matrix& other)
{}

void 
Matrix::init(matrixsize_t row, matrixsize_t col)
{
  if (row == _row && col == _col) return;
  clean();
  _row = row;
  _col = col;

  _data = new matrixdata_t*[_row];
  for (matrixsize_t i = 0; i < _row; i += 1)
  {
    _data[i] = new matrixdata_t[_col];
  }
}

void 
Matrix::set(matrixsize_t row, matrixsize_t col, matrixdata_t data)
{
  _data[row][col] = data;
}

void 
Matrix::print(std::ostream& os)
{
  std::_Ios_Fmtflags flgs = os.flags();
  os.setf(std::ios_base::right);
  os.setf(std::ios_base::fixed);
  os << "[" << std::endl;
  for (matrixsize_t r = 0; r < _row; r += 1)
  {
    os << " ";
    for (matrixsize_t c = 0; c < _col; c += 1)
    {
      os << " " << std::setw(6) << std::setprecision(2) << _data[r][c];
    }
    os << ";" << std::endl;
  }
  os << "]" << std::endl;
  os.setf(flgs);
}
