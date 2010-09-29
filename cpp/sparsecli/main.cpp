//#include <QtCore/QCoreApplication>

#include <libsparse/libsparse.h>
#include <vigra/regression.hxx>
#include <vigra/random.hxx>
#include <iostream>


using namespace vigra;
using namespace vigra::linalg;

void update_dict(Matrix<double>& D, Matrix<double> A, Matrix<double> B) {
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

double matrix_sum(Matrix<double>& A) {
    double result = 0;
    for(int i=0; i< A.rowCount(); i++)
        result += A(i,0);
    return result;
}

Matrix<double> lasso(Matrix<double> x, Matrix<double> D) {

    // arrays to hold the output
    ArrayVector<ArrayVector<int> > active_sets;
    ArrayVector<Matrix<double> > lasso_solutions;


//    std::cout << "prepare done" << std::endl;

    // run leastAngleRegression() in (non-negative) LASSO mode
    int numSolutions = leastAngleRegression(D, x, active_sets, lasso_solutions,
                                            LeastAngleRegressionOptions().nnlasso());


    int bestIndex = 0;
    double bestError = FLT_MAX;
    MultiArrayIndex k = 0;
    Matrix<double> dense_solution(D.columnCount(),1);

    for (k = 0; k < numSolutions; ++k)
    {
        dense_solution.init(0.0); // ensure that inactive variables are zero
        for (unsigned int i = 0; i < active_sets[k].size(); ++i)
        {
            // set the values of the active variables;
            // activeSets[k][i] is the true index of the i-th variable in the active set
            dense_solution(active_sets[k][i],0) = lasso_solutions[k](i,0);
        }

        double lsq = (mmul(D,dense_solution)-x).squaredNorm();
        double error = 0.5*lsq + 0.01*matrix_sum(lasso_solutions[k]);
//        std::cout << k << ": " << error << std::endl;
        if(error<bestError) { bestError=error; bestIndex=k; }
        // transform the sparse solution into a dense vector
//        // invert the input normalization
//        denseSolution = denseSolution * pointWise(scaling);

//        // output the solution
//        std::cout << "solution " << k << ":\n" << denseSolution << std::endl;
    }

//    std::cout << bestIndex << std::endl;

    dense_solution.init(0.0); // ensure that inactive variables are zero
    for (unsigned int i = 0; i < active_sets[bestIndex].size(); ++i)
    {
        // set the values of the active variables;
        // activeSets[k][i] is the true index of the i-th variable in the active set
        dense_solution(active_sets[bestIndex][i],0) = lasso_solutions[bestIndex](i,0);
    }
    return dense_solution;
}

Matrix<double> learn_dict(Matrix<double> X, int k) {
    int m = X.rowCount();
    int T = X.columnCount();
    Matrix<double> A(k, k), B(m, k), D(m,k);

    // fill D
    for (unsigned int i=0; i<m; i++) {
        for( unsigned int j=0; j<k; j++ )
            D(i,j) = rand();
    }
    prepareColumns(D, D, DataPreparationGoals(ZeroMean|UnitVariance));

    for(int t=0; t<T; t++) {

        Matrix<double> x = X.columnVector(t);

        //sparse code: min ....

        Matrix<double> a = lasso(x,D);
        std::cout << t << std::endl; // x <<  " " << D << "  " << a  << std::endl;

        A = A + mmul(a,a.transpose());
        B = B + mmul(x,a.transpose());
        update_dict(D,A,B);
    }
    return D;
}




int main(int argc, char *argv[])
{
    //    Libsparse* s = new Libsparse();
    int m = 192, n = 1200;
    Matrix<double> A(m, n), b(m, 1);

    // fill A and b
    for (unsigned int i=0; i<m; i++) {
        b(i,0) = rand();
//        std::cout << b(i,0) << std::endl;
        for( unsigned int j=0; j<n; j++ )
            A(i,j) = rand();
    }


    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(A, A, offset, scaling, DataPreparationGoals(ZeroMean|UnitVariance));
    prepareColumns(b, b, DataPreparationGoals(ZeroMean|UnitVariance));

    Matrix<double> D = learn_dict(A,100);

    Matrix<double> a = lasso(b,D);

    std::cout << D << "b: " << b << "a: " << a <<  std::endl;
}
