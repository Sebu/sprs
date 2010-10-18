#ifndef VIGRA_EXT_H
#define VIGRA_EXT_H

#include "regression.hxx"
#include <vigra/random.hxx>
#include <iostream>

using namespace vigra;
using namespace vigra::linalg;


template <class T, class C1, class C2, class C3>
bool linearSolveUpperTriangularC(const MultiArrayView<2, T, C1> &r, const MultiArrayView<2, T, C2> &b,
                                MultiArrayView<2, T, C3> x)
{
    typedef MultiArrayShape<2>::type Shape;
    MultiArrayIndex m = rowCount(r);
    MultiArrayIndex rhsCount = columnCount(b);
//    vigra_precondition(m == columnCount(r),
//        "linearSolveUpperTriangular(): square coefficient matrix required.");
//    vigra_precondition(m == rowCount(b) && m == rowCount(x) && rhsCount == columnCount(x),
//        "linearSolveUpperTriangular(): matrix shape mismatch.");

    for(MultiArrayIndex k = 0; k < rhsCount; ++k)
    {
        for(int i=m-1; i>=0; --i)
        {
            if(r(i,i) == NumericTraits<T>::zero())
                return false;  // r doesn't have full rank
            T sum = b(i, k);
            for(MultiArrayIndex j=i+1; j<m; ++j)
                 sum -= r(i, j) * x(j, k);
            x(i, k) = sum / r(i, i);
        }
    }
    return true;
}


template <class T, class C1, class C2, class C3>
bool linearSolveLowerTriangularC(const MultiArrayView<2, T, C1> &l, const MultiArrayView<2, T, C2> &b,
                            MultiArrayView<2, T, C3> x)
{
    MultiArrayIndex m = columnCount(l);
    MultiArrayIndex n = columnCount(b);
//    vigra_precondition(m == rowCount(l),
//        "linearSolveLowerTriangular(): square coefficient matrix required.");
//    vigra_precondition(m == rowCount(b) && m == rowCount(x) && n == columnCount(x),
//        "linearSolveLowerTriangular(): matrix shape mismatch.");

    for(MultiArrayIndex k = 0; k < n; ++k)
    {
        for(MultiArrayIndex i=0; i<m; ++i)
        {
            if(l(i,i) == NumericTraits<T>::zero())
                return false;  // l doesn't have full rank
            T sum = b(i, k);
            for(MultiArrayIndex j=0; j<i; ++j)
                 sum -= l(i, j) * x(j, k);
            x(i, k) = sum / l(i, i);
        }
    }
    return true;
}

template <class T, class C1, class C2, class C3>
inline
void choleskySolveC(MultiArrayView<2, T, C1> const & L, MultiArrayView<2, T, C2> const & b, MultiArrayView<2, T, C3> & x)
{
    /* Solve L * y = b */
    linearSolveLowerTriangularC(L, b, x);
    /* Solve L^T * x = y */
    linearSolveUpperTriangularC(transpose(L), x, x);
}

template<class T, class C>
T sum(MultiArrayView< 2, T, C > const & m) {
    T result = 0;
    for(int i=0; i<m.size(0); i++)
        for(int j=0; j<m.size(1); j++)
            result += m(i,j);

    return result;
}

template<class T, class C>
void init_random(MultiArrayView< 2, T, C >& m) {
    for(int i=0; i<m.size(0); i++)
        for(int j=0; j<m.size(1); j++)
            m(i,j) = rand();
}

int maxabs(Matrix<double>& c);
void vec_assign(Matrix<double>& y, Matrix<double>& x, Matrix<int>& ind, int k, int start);
Matrix<double> dense_vector(ArrayVector<int>  active_set, Matrix<double>  sparse_vector, int size);
Matrix<double> lasso(Matrix<double>& x, Matrix<double>& D);

#endif // VIGRA_EXT_H
