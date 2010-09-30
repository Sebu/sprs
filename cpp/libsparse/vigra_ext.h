#ifndef VIGRA_EXT_H
#define VIGRA_EXT_H

#include <vigra/regression.hxx>
#include <vigra/random.hxx>


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


Matrix<double> dense_vector(ArrayVector<int>  active_set, Matrix<double>  sparse_vector, int size) {
     Matrix<double> dense_vector(size,1);
     dense_vector.init(0.0);
     for (unsigned int i = 0; i < active_set.size(); ++i)
         dense_vector(active_set[i],0) = sparse_vector(i,0);

     return dense_vector;
}



#endif // VIGRA_EXT_H
