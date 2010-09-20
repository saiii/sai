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

#include <cstring>
#include <iomanip>
#include <stdexcept>

#include <math/Matrix.h>
#include <math/EigenSolver.h>

using namespace sai::math;

Matrix::Matrix():
  _eigenSolver(0),
  _row((matrixsize_t)0),
  _col((matrixsize_t)0),
  _data(0)
{
}

Matrix::Matrix(matrixsize_t row, matrixsize_t col):
  _eigenSolver(0),
  _row((matrixsize_t)0),
  _col((matrixsize_t)0),
  _data(0)
{
  init(row, col);
}

Matrix::~Matrix()
{
  clean();
  delete _eigenSolver;
}

void 
Matrix::clean()
{
  if (_data)
  {
    for (matrixsize_t i = 0; i < _row; i += 1)
    {
      delete [] _data[i];
    }
    delete [] _data;
    _data = 0;
  }
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
  if (_col != other._row )
  {
    throw std::invalid_argument("Invaid dimensions");
  }
  result.init(_row, other._col);

  matrixsize_t row = result._row;
  matrixsize_t col = result._col;
  matrixsize_t size = _col;

  for (matrixsize_t r = 0; r < row; r += 1)
  {
    for (matrixsize_t c = 0; c < col; c += 1)
    {
      matrixdata_t sum = (matrixdata_t) 0.0;
      for (matrixsize_t i = 0; i < size; i += 1)
      {
        sum += (_data[r][i] * other._data[i][c]);
      }
      result._data[r][c] = sum;
    }
  }
}

void 
Matrix::inverse(Matrix& other)
{
  // TODO
}

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

matrixdata_t 
Matrix::get(matrixsize_t row, matrixsize_t col)
{
  return _data[row][col];
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

    if (r + 1 < _row)
    {
      os << ";";
    }
    os << std::endl;
  }
  os << "]" << std::endl;
  os.setf(flgs);
}

EigenSolver* 
Matrix::getEigenSolver()
{
  if (!_eigenSolver)
  {
    _eigenSolver = new EigenSolver(this);
  }

  return _eigenSolver;
}
