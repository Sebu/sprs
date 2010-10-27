#include "samples.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "dictionary.h"
#include "vigra/matrix.hxx"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>

Samples::Samples() : data_(0) //, scaling_(0)
{
}

MatrixXf & Samples::getData() {
    return *data_;
}

//void Samples::saveImage(std::string& fileName, Dictionary& dict) {

//    CoderOMP coder;
//    std::cout << "restore image" << std::endl;
//    MatrixXf A = coder.encode((*data_), dict);
//    MatrixXf recon_vigra = dict.getData()*A;


//    std::cout << "reorder image" << std::endl;

//    cv::Mat outputImage(imageRows_+8, imageCols_+8, CV_8UC(channels_));
//    cv::Mat recon_cv(1,rows_,CV_8U);
//    int index = 0;//  ceil((float)j)*ceil((float)imageCols_) + ceil((float)i);
//    for(int j=0; j<imageRows_; j+=winSize_) {
//        for(int i=0; i<imageCols_; i+=winSize_) {
////            std::cout << index << " " << i <<  " " << j <<  std::endl;
//            for(int ii=0; ii<rows_; ii++)
//                recon_cv.at<uchar>(0,ii) = cv::saturate_cast<uchar>(recon_vigra(ii,index)); // /(*(scaling_))(0,index));
//            cv::Mat tmp = recon_cv.reshape(channels_, winSize_);
//            cv::Mat region( outputImage,  cv::Rect(i,j,winSize_, winSize_) );
//            tmp.copyTo(region);
//            index++;
//        }
//    }

//    cv::imwrite(fileName , outputImage);
//}

bool Samples::loadImage(std::string& fileName, int winSize, int channels, int step) {
    cv::Mat inputImage;

    channels_ = channels;
    winSize_ = winSize;
    switch(channels_) {
    case 1:
       inputImage = cv::imread(fileName, 0);
       break;
    default:
       inputImage = cv::imread(fileName);
       break;

    }

    if( inputImage.data==NULL ) {
        std::cout << "can't read image" << std::endl;
        return false;
    }

    imageRows_ = inputImage.rows;
    imageCols_ = inputImage.cols;
    rows_ = winSize_*winSize_*channels_;
    cols_ = ceil((float)imageRows_/(float)step) * ceil((float)imageCols_/float(step));
    if(data_) delete data_;
    data_ = new MatrixXf(rows_, cols_);

    int index = 0;
    for(int j=0; j<imageRows_; j+=step) {
        for(int i=0; i<imageCols_; i+=step) {
            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<double>(0,2)=-i;
            transMat.at<double>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(inputImage, warped, transMat, cv::Size(winSize_, winSize_));
            cv::Mat tmp = warped.reshape(1,1);
            for(int ii=0; ii<tmp.cols; ii++) {
                (*data_)(ii,index) = tmp.at<uchar>(0,ii);
            }
            index++;
        }
    }
    // normalize the input
//    vigra::Matrix<double> offset(1,cols_);
//    scaling_ = new vigra::Matrix<double>(1,cols_);
//    prepareColumns((*data_), (*data_), offset, (*scaling_), vigra::linalg::DataPreparationGoals(vigra::linalg::UnitNorm));
    return true;
}
