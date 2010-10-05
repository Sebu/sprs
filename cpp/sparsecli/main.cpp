//#include <QtCore/QCoreApplication>

#include <libsparse/libsparse.h>
#include <libsparse/regression.hxx>
#include <vigra/random.hxx>

#include <libsparse/vigra_ext.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>


using namespace vigra;
using namespace vigra::linalg;

void update_dict(Matrix<double>& D, Matrix<double>& A, Matrix<double>& B) {

#pragma omp parallel for
    for(int i=0; i < 10; i++) {
        for(int j=0; j < D.columnCount(); j++) {
            Matrix<double> a = A.columnVector(j);
            if(A(j,j)==0.0) continue;
            Matrix<double> b = B.columnVector(j);
            Matrix<double> d = D.columnVector(j);
            Matrix<double> u = ( (1.0/A(j,j)) * (b-(D*a)) ) + d;
            Matrix<double> tmp = (1.0/fmax(u.norm(),0.0)) * u;
            for(int i=0; i< D.rowCount(); i++)
                D(i,j) = tmp(i,0);
        }
    }

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

    for (MultiArrayIndex k = 0; k < numSolutions; ++k) {
        Matrix<double> dense_solution = dense_vector(active_sets[k], solutions[k], D.columnCount());
        double lsq = (mmul(D,dense_solution)-x).squaredNorm();
        //        std::cout <<  k << " " << sum(solutions[k]) <<  " " << solutions[k].size() << std::endl;
        double error = 0.5*lsq + sum(solutions[k]);
        if(error<bestError) { bestError=error; bestIndex=k; }
    }

    Matrix<double> bla = dense_vector(active_sets[bestIndex], solutions[bestIndex], D.columnCount());;
    return bla;
}


Matrix<double> learn_dict(Matrix<double>& samples, int dict_size, int iterations) {


    int m = samples.rowCount();

    Matrix<double> A(dict_size, dict_size), B(m, dict_size), D(m, dict_size);

    // init A,B with 0
    A.init(0.0); B.init(0.0);

    // fill D with random start data
    init_random(D);
    prepareColumns(D, D, DataPreparationGoals(UnitNorm));

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

    cv::Mat inputImage = cv::imread("/home/seb/Bilder/bild5.jpg");

    int size = 8;
    int m = size*size*3;
    int selectionId = 1230;

    int rowMax = inputImage.rows;
    int colMax = inputImage.cols;
    int n = (rowMax * colMax) / (8*8);

    Matrix<double> training_set(m, n);\

            int index = 0;
    for(int j=0; j<rowMax-size; j+=size) {
        for(int i=0; i<colMax-size; i+=size) {
            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<double>(0,2)=-i;
            transMat.at<double>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(inputImage, warped, transMat, cv::Size(size, size));
            cv::Mat tmp = warped.reshape(1,1);
            std::cout << index << std::endl;
            for(int ii=0; ii<m; ii++) {
                training_set(ii,index) = tmp.at<uchar>(0,ii);
            }

            index++;
        }
    }

    std::cout << "train set fill complete " <<  index << std::endl;

    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(training_set, training_set,offset, scaling, DataPreparationGoals(UnitNorm));

    Matrix<double> D = learn_dict(training_set, 400, 2000);


    cv::Mat outputImage(rowMax,colMax,CV_8UC3);

    index = 0;
    for(int j=100; j<200; j+=size) {
        for(int i=150; i<200; i+=size) {

            Matrix<double> signal = training_set.columnVector(index);
            Matrix<double> a = lasso(signal, D);
            Matrix<double> recon_vigra = D*a;
            cv::Mat recon_cv(1,m,CV_8U);
            for(int ii=0; ii<m; ii++)
                recon_cv.at<uchar>(0,ii) = (uchar)(recon_vigra(ii,0)/scaling(0,index));
            cv::Mat tmp = recon_cv.reshape(3, size);
            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
            tmp.copyTo(region);

            index++;
        }
    }

    //    std::cout << D << "b: " << signal << "a: " << a <<  std::endl;


    cv::imwrite("/home/seb/Bilder/recon_lasso.jpg", outputImage);

}
