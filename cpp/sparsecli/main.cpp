//#include <QtCore/QCoreApplication>

#include <libsparse/libsparse.h>
#include <vigra/regression.hxx>
#include <vigra/random.hxx>
#include <iostream>

using namespace vigra;
using namespace vigra::linalg;

void update_dict(Matrix<double> D, Matrix<double> A, Matrix<double> B) {
    for(int j=0; j < D.columnCount(); j++) {
        Matrix<double> a = A.columnVector(j);
        Matrix<double> b = B.columnVector(j);
        Matrix<double> d = D.columnVector(j);
        Matrix<double> u = (1/A(j,j)) * (b-D*a) + d;
        d = (1/fmax(u.norm(),0)) * u;
        //        Matrix<double> tmp = (1/ fmax(u.norm(),0) )*u;
        //        for (int i=0; i<tmp.rowCount(); i++)
        //            D(j,i) = tmp(i,0);
    }
}

void learn_dict(Matrix<double> X, int k) {
    int m = X.rowCount();
    int T = X.columnCount();
    Matrix<double> A(k, k), B(m, k), D(m,k);
    for(int t=0; t<T; t++) {
        Matrix<double> x = X.columnVector(t);

        //TODO: sparse code: min ....
        a = lasso(x,D);
        Matrix<double> a(T,1);

        A = A + a*a.transpose();
        B = B + x*a.transpose();
        update_dict(D,A,B);
    }
}



void lasso(Matrix<double> x, Matrix<double> D) {

    // arrays to hold the output
    ArrayVector<ArrayVector<int> > activeSets;
    ArrayVector<Matrix<double> > lasso_solutions;
    ArrayVector<Matrix<double> > lsq_solutions;


//    std::cout << "prepare done" << std::endl;

    // run leastAngleRegression() in (non-negative) LASSO mode
    int numSolutions = leastAngleRegression(D, x, activeSets, lasso_solutions, lsq_solutions,
                                            LeastAngleRegressionOptions().lasso());


    // print results
//    Matrix<double> denseSolution(1, n);

    int bestIndex = 0;
    double bestError = MAX_FLT;

    for (MultiArrayIndex k = 0; k < numSolutions; k++)
    {
        double error = (1/2)*lsq_solutions[k](0,0) + lasso_solutions[k].sum();
        if(error<bestError) { bestError=error; bestIndex=k; }
//        // transform the sparse solution into a dense vector
//    //    denseSolution.init(0.0); // ensure that inactive variables are zero
//        for (unsigned int i = 0; i < activeSets[k].size(); ++i)
//        {
//            // set the values of the active variables;
//            // activeSets[k][i] is the true index of the i-th variable in the active set
//            denseSolution(0, activeSets[k][i]) = solutions[k](i,0);
//        }

//        // invert the input normalization
//        denseSolution = denseSolution * pointWise(scaling);

//        // output the solution
//        std::cout << "solution " << k << ":\n" << denseSolution << std::endl;
    }
}

int main(int argc, char *argv[])
{
    //    Libsparse* s = new Libsparse();
    int m = 4, n = 12;
    Matrix<double> A(m, n), b(m, 1);

    // fill A and b
    for (unsigned int i=0; i<m; i++) {
        b(i,0) = rand();
        std::cout << b(i,0) << std::endl;
        for( unsigned int j=0; j<n; j++ )
            A(i,j) = rand();
    }


    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(A, A, offset, scaling, DataPreparationGoals(ZeroMean|UnitVariance));
    prepareColumns(b, b, DataPreparationGoals(ZeroMean));
    lasso(b,A);
}
