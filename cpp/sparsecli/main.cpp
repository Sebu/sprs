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

    cv::Mat inputImage = cv::imread("/homes/wheel/seb/Bilder/lena.png",0);

    int size = 12;
    int m = size*size;

    int rowMax = inputImage.rows;
    int colMax = inputImage.cols;
    int n = ceil((float)rowMax) * ceil((float)colMax);

    std::cout << n << std::endl;
    Matrix<double> training_set(m, n);


    int index = 0;
    for(int j=0; j<rowMax; j+=1) {
        for(int i=0; i<colMax; i+=1) {
            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<double>(0,2)=-i;
            transMat.at<double>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(inputImage, warped, transMat, cv::Size(size, size));
            cv::Mat tmp = warped.reshape(1,1);
            std::cout << index << std::endl;
            for(int ii=0; ii<tmp.cols; ii++) {
                training_set(ii,index) = tmp.at<uchar>(0,ii);
            }

            index++;
        }
    }
    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(training_set, training_set,offset, scaling, DataPreparationGoals(UnitNorm));

    std::cout << "train set fill complete " <<  index << std::endl;


    Dictionary dict(m, 225);

//    dict.initRandom();
    dict.initFromData(training_set);
    dict.learn(training_set, 1000);
    Matrix<double> D = dict.getData();

    prepareColumns(D, D, DataPreparationGoals(UnitNorm));

    cv::Mat outputImage(size*15,size*15,CV_8UC1);
    for(int j=0; j<size*15; j+=size) {
        for(int i=0; i<size*15; i+=size) {
            index = ceil(j/size)*15 + ceil(i/size);
            Matrix<double> d = D.columnVector(index);
            cv::Mat recon_cv(1,m,CV_8U);
            for(int ii=0; ii<m; ii++) {
                recon_cv.at<uchar>(0,ii) = (uchar)((d(ii,0)+1.0)*128.0);
            }
            cv::Mat tmp = recon_cv.reshape(1, size);
            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
            tmp.copyTo(region);
        }
    }
//    cv::Mat outputImage(rowMax,colMax,CV_8UC1);
//    for(int j=0; j<rowMax/2; j+=size) {
//        for(int i=0; i<colMax/2; i+=size) {
//            index = ceil((float)j/(float)size)*ceil((float)colMax/(float)size) + ceil((float)i/(float)size);
//            Matrix<double> signal = training_set.columnVector(index);
//            Matrix<double> a = lasso(signal, D);
//            Matrix<double> recon_vigra = D*a;
//            cv::Mat recon_cv(1,m,CV_8U);
//            for(int ii=0; ii<m; ii++)
//                recon_cv.at<uchar>(0,ii) = (uchar)(recon_vigra(ii,0)/scaling(0,index));
//            cv::Mat tmp = recon_cv.reshape(1, size);
//            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
//            tmp.copyTo(region);
//        }
//    }

    //    std::cout << D << "b: " << signal << "a: " << a <<  std::endl;

    cv::imwrite("/homes/wheel/seb/Bilder/dict_lasso.jpg", outputImage);

}
