//#include <QtCore/QCoreApplication>

#include <libsparse/libsparse.h>
#include <vigra/regression.hxx>
#include <vigra/random.hxx>
#include <iostream>

using namespace vigra;
using namespace vigra::linalg;

int main(int argc, char *argv[])
{
//    Libsparse* s = new Libsparse();


    int m = 4, n = 40;
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

    // arrays to hold the output
    ArrayVector<ArrayVector<int> > activeSets;
    ArrayVector<Matrix<double> > solutions;


    std::cout << "prepare done" << std::endl;

    // run leastAngleRegression() in non-negative LASSO mode
    int numSolutions = leastAngleRegression(A, b, activeSets, solutions,
                                LeastAngleRegressionOptions().lasso());


    // print results
    Matrix<double> denseSolution(1, n);

    for (MultiArrayIndex k = 0; k < numSolutions; ++k)
    {
        // transform the sparse solution into a dense vector
        denseSolution.init(0.0); // ensure that inactive variables are zero
        for (unsigned int i = 0; i < activeSets[k].size(); ++i)
        {
            // set the values of the active variables;
            // activeSets[k][i] is the true index of the i-th variable in the active set
            denseSolution(0, activeSets[k][i]) = solutions[k](i,0);
        }

        // invert the input normalization
        denseSolution = denseSolution * pointWise(scaling);

        // output the solution
        std::cout << "solution " << k << ":\n" << denseSolution << std::endl;
    }

}
