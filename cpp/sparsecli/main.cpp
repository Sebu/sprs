//#include <QtCore/QCoreApplication>

#include <libsparse/libsparse.h>
#include <vigra/regression.hxx>
#include <vigra/random.hxx>

#include <libsparse/vigra_ext.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>


using namespace vigra;
using namespace vigra::linalg;

void update_dict(Matrix<double>& D, Matrix<double>& A, Matrix<double>& B) {

    for(int j=0; j < D.columnCount(); j++) {
        Matrix<double> a = A.columnVector(j);
        Matrix<double> b = B.columnVector(j);
        Matrix<double> d = D.columnVector(j);
        Matrix<double> u = (1/A(j,j)) * (b-(D*a)) + d;
        Matrix<double> tmp = (1/fmax(u.norm(),0)) * u;
        std::cout << a << b << d << std::endl;
        for(int i=0; i< D.rowCount(); i++)
            D(i,j) = tmp(i,0);
    }
}


Matrix<double> lasso(Matrix<double>& x, Matrix<double>& D) {

    int bestIndex = 0;
    double bestError = FLT_MAX;
    ArrayVector<ArrayVector<int> > active_sets;
    ArrayVector<Matrix<double> > solutions;


    std::cout << D << " " << x << std::endl;
    // run leastAngleRegression() in  LASSO mode
    int numSolutions = leastAngleRegression(D, x, active_sets, solutions,
                                            LeastAngleRegressionOptions().lasso());

    for (MultiArrayIndex k = 0; k < numSolutions; ++k) {
        Matrix<double> dense_solution = dense_vector(active_sets[k], solutions[k], D.columnCount());
        double lsq = (mmul(D,dense_solution)-x).squaredNorm();
        double error = 0.5*lsq + sum(solutions[k]);
        if(error<bestError) { bestError=error; bestIndex=k; }
    }
    Matrix<double> bla = dense_vector(active_sets[bestIndex], solutions[bestIndex], D.columnCount());;
    return bla;
}


Matrix<double> learn_dict(Matrix<double>& samples, int dict_size) {


    int m = samples.rowCount();
    int iterations = samples.columnCount();

    Matrix<double> A(dict_size, dict_size), B(m, dict_size), D(m, dict_size);

    // init A,B with 0
    A.init(0.0); B.init(0.0);

    // fill D with random start data
    init_random(D);
    prepareColumns(D, D, DataPreparationGoals(UnitNorm)); //ZeroMean|UnitVariance));

    for(int t=0; t<iterations; t++) {

        // draw sample from trainig set
        Matrix<double> sample = samples.columnVector(t);

        // sparse code sample
        Matrix<double> a = lasso(sample,D);

        std::cout << "Iteration: " <<  t << std::endl;

        A = A + mmul(a,a.transpose());
        B = B + mmul(sample,a.transpose());
        // update step (algo. 2)
        update_dict(D,A,B);
    }

    return D;
}




int main(int argc, char *argv[])
{

    cv::Mat image = cv::imread("/homes/wheel/seb/Bilder/lena.jpg");

    int m = 4;

    int row_max = 10; // image.rows;
    int col_max = 10; // image.cols;
    int n = row_max * col_max;

    Matrix<double> training_set(m, n); // signal(m, 1);

    int index = 0;
    for(int j=0; j<row_max; j++) {
        for(int i=0; i<col_max; i++) {
            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<float>(0,2)=-i;
            transMat.at<float>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(image, warped, transMat, cv::Size(8, 8));
            cv::Mat tmp = warped.reshape(1,1);

            for(int ii=0; ii<m; ii++) {
                training_set(ii,index) = tmp.at<uchar>(0,ii);
            }

            index++;
        }
    }

    std::cout << "train set fill complete" << std::endl;

    // fill A and b
    //    init_random(signal);
    //    init_random(trainig_set);


    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(training_set, training_set,offset, scaling, DataPreparationGoals(UnitNorm));
    //    prepareColumns(signal, signal,  DataPreparationGoals(UnitNorm));

    Matrix<double> D = learn_dict(training_set, 12);

    Matrix<double> signal = training_set.columnVector(1);
    Matrix<double> a = lasso(signal, D);

    std::cout << D << "b: " << signal << "a: " << a <<  std::endl;
}
