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

#include <math/Types.h>
#include <math/EigenSolver.h>
#include <math/Matrix.h>
#include <math/Vector.h>

namespace sai 
{
namespace math
{
  class EigenSolverImpl
  {
    protected:
      VectorList     _vectors;
      MatrixDataList _values;

    public:
      EigenSolverImpl() {}
      virtual ~EigenSolverImpl() { clean(); }
      virtual void solve() = 0;
      virtual VectorList&     getEigenVectors() = 0;
      virtual MatrixDataList& getEigenValues() = 0;

      void clean();
  };

  class PowerMethod : public EigenSolverImpl
  {
    private:
      Matrix        *_matrix;

    public:
      PowerMethod(Matrix * matrix);
      ~PowerMethod();

      void            solve();
      VectorList&     getEigenVectors();
      MatrixDataList& getEigenValues();
  };

  class ShiftedInversePowerMethod : public EigenSolverImpl
  {
    // TODO
  };

  class JacobiMethod : public EigenSolverImpl
  {
    // TODO
  };

  class QLMethod : public EigenSolverImpl
  {
    // TODO
  };
}
}

using namespace sai::math;

EigenSolver::EigenSolver(Matrix *matrix):
  _impl(0),
  _matrix(matrix)
{}

EigenSolver::~EigenSolver()
{
  delete _impl;
}

void 
EigenSolver::configure()
{
  if (_matrix->getRow() != _matrix->getCol())
  {
    return;
  }

  // TODO

  _impl = new PowerMethod(_matrix);
}

void            
EigenSolver::solve()
{
  _impl->solve();
}

VectorList&     
EigenSolver::getEigenVectors()
{
  return _impl->getEigenVectors();
}

MatrixDataList& 
EigenSolver::getEigenValues()
{
  return _impl->getEigenValues();
}

//-----------------------------------------------------------------------------

void 
EigenSolverImpl::clean()
{
  while (_vectors.size() > 0)
  {
    Vector * vector = _vectors.front();
    _vectors.erase(_vectors.begin());
    delete vector;
  }

  _values.clear();
}

PowerMethod::PowerMethod(Matrix *matrix):
  _matrix(matrix)
{}

PowerMethod::~PowerMethod()
{}

void 
PowerMethod::solve()
{
#if 0
  if (_vectors.size() > 0 || _values.size() > 0)
  {
    clean();
  }

  const matrixsize_t MAX = 3000;

  Vector vector(_matrix->getRow(), 1);
  vector = (matrixdata_t) 1.0;

  matrixdata_t value = (matrixdata_t) 0.0;

  Vector tmp(_matrix->getRow(), 1);
  tmp = (matrixdata_t) 0.0;

  for (matrixsize_t i = 0; i < MAX; i += 1)
  {
    _matrix->multiply(vector, tmp);
    vector = tmp;
  }
  matrixdata_t min = (matrixdata_t) 0.0;
  vector.min(&min);

  vector *= (matrixdata_t)(1.0/min);

  _matrix->multiply(vector, tmp);
  matrixdata_t v1, v2;
  v1 = v2 = (matrixdata_t) 0.0;
  for (matrixsize_t j = 0; j < vector.getRow(); j += 1)
  {
    v1 += (vector.get(j, 0) * tmp.get(j, 0));
    v2 += (vector.get(j, 0) * vector.get(j, 0));
  }
  value = v1 / v2;
  
  Vector * result = new Vector(_matrix->getRow(), 1);
  result->operator=(vector);

  _vectors.push_back(result);
  _values.push_back(value);
#endif
  if (_vectors.size() > 0 || _values.size() > 0)
  {
    clean();
  }

  const matrixdata_t ERR = (matrixdata_t) 0.00000001;
  const matrixsize_t MAX = 30;

  matrixdata_t error = std::numeric_limits<matrixdata_t>::max();

  Vector vector(_matrix->getRow(), 1);
  vector = (matrixdata_t) 1.0;

  matrixdata_t value = (matrixdata_t) 0.0;

  Vector tmp(_matrix->getRow(), 1);
  tmp = (matrixdata_t) 0.0;

  matrixsize_t i;
  for (i = 0; i < MAX && error > ERR; i += 1)
  {
    _matrix->multiply(vector, tmp);

    matrixdata_t max = (matrixdata_t) 0.0;
    tmp.max(&max);

    matrixdata_t diff1 = abs(value - max);
    tmp *= (matrixdata_t)(1.0/max);

    matrixdata_t diff2 = vector.dist(tmp);
    error = diff1 > diff2 ? diff1 : diff2;

    vector = tmp;
    value  = max;
  }

  Vector * result = new Vector(_matrix->getRow(), 1);
  result->operator=(vector);

  _vectors.push_back(result);
  _values.push_back(value);
}

VectorList& 
PowerMethod::getEigenVectors()
{
  return _vectors;
}

MatrixDataList& 
PowerMethod::getEigenValues()
{
  return _values;
}

