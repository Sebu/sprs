#include "samples.h"
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
void Samples::load(std::string& fileName, int winSize, int channels) {
    cv::Mat inputImage = cv::imread(fileName, 0);

    int m = winSize*winSize*channels;

    rowMax = inputImage.rows;
    colMax = inputImage.cols;
    int n = ceil((float)rowMax) * ceil((float)colMax);

    std::cout << n << std::endl;
    data_ = new vigra::Matrix<double>(m, n);

    int index = 0;
    for(int j=0; j<rowMax; j+=1) {
        for(int i=0; i<colMax; i+=1) {
            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<double>(0,2)=-i;
            transMat.at<double>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(inputImage, warped, transMat, cv::Size(winSize, winSize));
            cv::Mat tmp = warped.reshape(1,1);
            std::cout << index << std::endl;
            for(int ii=0; ii<tmp.cols; ii++) {
                (*data_)(ii,index) = tmp.at<uchar>(0,ii);
            }
            index++;
        }
    }
    // normalize the input
    vigra::Matrix<double> offset(1,n);
    scaling_ = new vigra::Matrix<double>(1,n);
    prepareColumns((*data_), (*data_), offset, (*scaling_), vigra::linalg::DataPreparationGoals(vigra::linalg::UnitNorm));
}
