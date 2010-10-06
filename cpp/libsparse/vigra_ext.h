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


Matrix<double> dense_vector(ArrayVector<int>  active_set, Matrix<double>  sparse_vector, int size) {

    Matrix<double> dense_vector(size,1);
    dense_vector.init(0.0);
    for (unsigned int i = 0; i < active_set.size(); i++)
        dense_vector(active_set[i],0) = sparse_vector(i,0);

    return dense_vector;
}


Matrix<double> lasso(Matrix<double>& x, Matrix<double>& D) {

    int bestIndex = 0;
    double bestError = FLT_MAX;
    ArrayVector<ArrayVector<int> > active_sets;
    ArrayVector<Matrix<double> > solutions;


    LeastAngleRegressionOptions opts;
    opts.lasso();
    opts.maxSolutionCount(10);
    // run leastAngleRegression() in  LASSO mode
    int numSolutions = leastAngleRegression(D, x, active_sets, solutions, opts);


    //std::cout << bestIndex << std::endl;

//    for (MultiArrayIndex k = 0; k < numSolutions; ++k) {
//        Matrix<double> dense_solution = dense_vector(active_sets[k], solutions[k], D.columnCount());
//        double lsq = (mmul(D,dense_solution)-x).squaredNorm();
//        //        std::cout <<  k << " " << sum(solutions[k]) <<  " " << solutions[k].size() << std::endl;
//        double error = 0.5*lsq + sum(solutions[k]);
//        if(error<bestError) { bestError=error; bestIndex=k; }
//    }

    bestIndex = numSolutions - 1;
    return dense_vector(active_sets[bestIndex], solutions[bestIndex], D.columnCount());
}


#endif // VIGRA_EXT_H
