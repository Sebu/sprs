#include "samples.h"
#include "coderlasso.h"
#include "dictionary.h"
#include "vigra/matrix.hxx"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>

Samples::Samples() : data_(0), scaling_(0)
{
}

vigra::Matrix<double> & Samples::getData() {
    return *data_;
}

void Samples::save(std::string& fileName, Dictionary& dict) {

    CoderLasso coder;
//    vigra::Matrix<double> tmp(rows_,cols_);
//    for(int i=0; i<cols_; i+=1000)
//    {
//        vigra::Matrix<double> a = coder.code(signal, dict);
//        vigra::Matrix<double> recon_vigra = dict.getData()*a;
//    }

    std::cout << "restore image" << std::endl;
    cv::Mat outputImage(imageRows_, imageCols_, CV_8UC(channels_));
    for(int j=0; j<imageRows_; j+=winSize_) {
        for(int i=0; i<imageCols_; i+=winSize_) {
            int index = ceil((float)j)*ceil((float)imageCols_) + ceil((float)i);
            vigra::Matrix<double> signal = (*data_).columnVector(index);
            vigra::Matrix<double> a = coder.code(signal, dict);
            vigra::Matrix<double> recon_vigra = dict.getData()*a;
            cv::Mat recon_cv(1,rows_,CV_8U);
            for(int ii=0; ii<rows_; ii++)
                recon_cv.at<uchar>(0,ii) = cv::saturate_cast<uchar>(recon_vigra(ii,0)); // /(*(scaling_))(0,index));
            cv::Mat tmp = recon_cv.reshape(channels_, winSize_);
            cv::Mat region( outputImage,  cv::Rect(i,j,winSize_, winSize_) );
            tmp.copyTo(region);
        }
    }

    cv::imwrite(fileName , outputImage);
}

void Samples::load(std::string& fileName, int winSize, int channels) {
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

    imageRows_ = inputImage.rows;
    imageCols_ = inputImage.cols;
    rows_ = winSize_*winSize_*channels_;
    cols_ = ceil((float)imageRows_) * ceil((float)imageCols_);
    data_ = new vigra::Matrix<double>(rows_, cols_);

    std::cout << imageRows_ << std::endl;
    int index = 0;
    for(int j=0; j<imageRows_; j+=1) {
        for(int i=0; i<imageCols_; i+=1) {
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
    vigra::Matrix<double> offset(1,cols_);
    scaling_ = new vigra::Matrix<double>(1,cols_);
//    prepareColumns((*data_), (*data_), offset, (*scaling_), vigra::linalg::DataPreparationGoals(vigra::linalg::UnitNorm));
}
