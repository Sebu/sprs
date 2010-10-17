#ifndef VIGRA_EXT_H
#define VIGRA_EXT_H

#include "regression.hxx"
#include <vigra/random.hxx>
#include <iostream>

using namespace vigra;
using namespace vigra::linalg;

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
