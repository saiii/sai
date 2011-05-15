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
#include <math/Distance.h>

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
      virtual VectorList&     getEigenVectors() { return _vectors; }
      virtual MatrixDataList& getEigenValues()  { return _values;  }

      void clean();
  };

  class PowerMethod : public EigenSolverImpl
  {
    private:
      Matrix *_matrix;

    public:
      PowerMethod(Matrix * matrix);
      ~PowerMethod();

      void solve();
  };

  class JacobiMethod : public EigenSolverImpl
  {
    private:
      Matrix *_matrix;

    public:
      JacobiMethod(Matrix * matrix);
      ~JacobiMethod();

      void solve();
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
EigenSolver::configure(EigenSolverProperty prop)
{
  if (_matrix->getRow() != _matrix->getCol())
  {
    return;
  }

  if (_impl)
  {
    delete _impl;
  }

  switch (prop)
  {
    case ONE_POWERMETHOD:
      _impl = new PowerMethod(_matrix);
      break;
    case ALL_JACOBIMETHOD:
      _impl = new JacobiMethod(_matrix);
      break;
    default:
      return;
  }
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

JacobiMethod::JacobiMethod(Matrix * matrix):
  _matrix(matrix)
{
}

JacobiMethod::~JacobiMethod()
{
}

void 
JacobiMethod::solve()
{
  if (_vectors.size() > 0 || _values.size() > 0)
  {
    clean();
  }

  matrixsize_t dim = _matrix->getRow();
  Matrix values(dim, dim);
  values = *_matrix;
  Matrix vectors(dim, dim);
  vectors = (matrixdata_t) 0.0;
  for (matrixsize_t i = 0; i < dim; i += 1)
  {
    vectors.set(i, i, (matrixdata_t) 1.0);
  }

  const matrixsize_t MAX = 50;
  const matrixdata_t THRESHOLD = 0.000000000001;

  matrixsize_t d2 = dim - 1;
  Vector xj(dim, 1), xk(dim, 1);
  Vector yj(dim, 1), yk(dim, 1);
  matrixdata_t err = std::numeric_limits<matrixdata_t>::max();
  for (matrixsize_t i = 0; i < MAX; i += 1)
  {
    for (matrixsize_t j = 0; j < d2; j += 1)
    {
      for (matrixsize_t k = j + 1; k < dim; k += 1)
      {
        matrixdata_t theta = (values.get(k, k) - values.get(j, j))/
                             ((matrixdata_t)2.0 * values.get(j, k));
        matrixdata_t t = 1.0 / (fabs(theta) + sqrt((theta * theta) + 1.0));
        if (theta < 0.0) 
        {
          t *= -1.0;
        }
        matrixdata_t c = 1.0 / (sqrt((t * t) + 1.0));
        matrixdata_t s = c * t;

        for (matrixsize_t l = 0; l < dim; l += 1)
        {
          xj.set(l, 0, values.get(l, j));
          xk.set(l, 0, values.get(l, k));
        }

        values.set(j, k, (matrixdata_t)0.0);
        values.set(k, j, (matrixdata_t)0.0);
        values.set(j, j, ((c * c * xj.get(j, 0)) + (s * s * xk.get(k, 0)) - (2.0 * c * s * xk.get(j, 0))));
        values.set(k, k, ((s * s * xj.get(j, 0)) + (c * c * xk.get(k, 0)) + (2.0 * c * s * xk.get(j, 0))));

        for (matrixsize_t l = 0; l < dim; l += 1)
        {
          if (l != j && l != k)
          {
            values.set(l, j, ((c * xj.get(l, 0)) - (s * xk.get(l, 0))));
            values.set(j, l, values.get(l, j));

            values.set(l, k, ((c * xk.get(l, 0)) + (s * xj.get(l, 0))));
            values.set(k, l, values.get(l, k));
          }
        }

        for (matrixsize_t l = 0; l < dim; l += 1)
        {
          yj.set(l, 0, vectors.get(l, j));
          yk.set(l, 0, vectors.get(l, k));
        }

        for (matrixsize_t l = 0; l < dim; l += 1)
        {
          vectors.set(l, j, (c * yj.get(l,0)) - (s * yk.get(l,0)));
          vectors.set(l, k, (s * yj.get(l,0)) + (c * yk.get(l,0)));
        }
      } // Loop k
    } // Loop j

    matrixdata_t sum = (matrixdata_t)0.0;
    for (matrixsize_t ind = 0; ind < dim; ind += 1)
    {
      matrixdata_t val = values.get(ind, ind);
      sum += (val * val);
    }
    sum = sqrt(sum);
    if (fabs(err - sum) < THRESHOLD)
    {
      break;
    }
    err = sum;
  } // Main Loop

  // keep values
  for (matrixsize_t i = 0; i < dim; i += 1)
  {
    _values.push_back(values.get(i,i));
  }

  // keep vectors
  for (matrixsize_t i = 0; i < dim; i += 1)
  {
    Vector * result = new Vector(dim, 1);
    for (matrixsize_t j = 0; j < dim; j += 1)
    {
      result->set(j, 0, vectors.get(j, i));
    }
    _vectors.push_back(result);
  }
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
  Distance * distance = DistanceCreator().create(SAI_AI_DIST_EUCLIDEAN);

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

    matrixdata_t diff1 = fabs(value - max);
    tmp *= (matrixdata_t)(1.0/max);

    matrixdata_t diff2 = distance->dist(vector, tmp);
    error = diff1 > diff2 ? diff1 : diff2;

    vector = tmp;
    value  = max;
  }

  Vector * result = new Vector(_matrix->getRow(), 1);
  result->operator=(vector);

  _vectors.push_back(result);
  _values.push_back(value);

  delete distance;
}

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

