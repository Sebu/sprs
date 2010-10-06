//#include <QtCore/QCoreApplication>

#include <libsparse/dictionary.h>
#include <libsparse/regression.hxx>
#include <vigra/random.hxx>

#include <libsparse/vigra_ext.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>


using namespace vigra;
using namespace vigra::linalg;










int main(int argc, char *argv[])
{

    cv::Mat inputImage = cv::imread("/home/seb/Bilder/bild5.jpg");

    int size = 8;
    int m = size*size*3;

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
    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(training_set, training_set,offset, scaling, DataPreparationGoals(UnitNorm));

    std::cout << "train set fill complete " <<  index << std::endl;


    Dictionary dict(m, 400);
    dict.learn(training_set, 100);
    Matrix<double> D = dict.getData();

    cv::Mat outputImage(rowMax,colMax,CV_8UC3);

    index = 0;
    for(int j=0; j<(rowMax-size); j+=size) {
        for(int i=0; i<(colMax-size); i+=size) {
            index = (j/8)*((colMax-size)/8) + (i/8);
            Matrix<double> signal = training_set.columnVector(index);
            Matrix<double> a = lasso(signal, D);
            Matrix<double> recon_vigra = D*a;
            cv::Mat recon_cv(1,m,CV_8U);
            for(int ii=0; ii<m; ii++)
                recon_cv.at<uchar>(0,ii) = (uchar)(recon_vigra(ii,0)/scaling(0,index));
            cv::Mat tmp = recon_cv.reshape(3, size);
            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
            tmp.copyTo(region);
        }
    }

    //    std::cout << D << "b: " << signal << "a: " << a <<  std::endl;


    cv::imwrite("/home/seb/Bilder/recon_lasso.jpg", outputImage);

}
