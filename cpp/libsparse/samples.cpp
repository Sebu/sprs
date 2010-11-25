#include "samples.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "dictionary.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <fstream>
#include "vigra_ext.h"

Samples::Samples() : data_(0) //, scaling_(0)
{
}

MatrixXd & Samples::getData() {
    return *data_;
}

void Samples::saveImage(std::string& fileName, Dictionary& dict) {

    double quant = 1.0;
    CoderLasso coder;
    std::cout << "restore image" << std::endl;
    //center((*data_));
    Eigen::SparseMatrix<double> A = coder.encode((*data_), dict);

    int count=0;
    std::ofstream ofs( (fileName + ".sp").c_str(), std::ios::out | std::ios::binary );
    for (int k=0; k<A.outerSize(); ++k)
      for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {
          short data=0;
          data = (short)round(it.row());
          ofs.write((char*)&data,sizeof(data));
          data = (short)round(it.value()/quant);
          A.coeffRef(it.row(),it.col()) = it.value(); //data;
          ofs.write((char*)&data,sizeof(char));
          if (data>count) count = data;
      }
    ofs.close();

    std::cout << count << std::endl;
    MatrixXd recon_vigra = dict.getData()*A;


    std::cout << "reorder image" << std::endl;

    cv::Mat outputImage(imageRows_+8, imageCols_+8, CV_8UC(channels_));
    cv::Mat recon_cv(1,rows_,CV_8U);
    int index = 0;//  ceil((float)j)*ceil((float)imageCols_) + ceil((float)i);
    for(int j=0; j<imageRows_; j+=winSize_) {
        for(int i=0; i<imageCols_; i+=winSize_) {
//            std::cout << index << " " << i <<  " " << j <<  std::endl;
            for(int jj=0; jj<channels_; jj++){
                for(int ii=0; ii<rows_/channels_; ii++) {
                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>(recon_vigra(jj*(rows_/channels_)+ii, index)*quant);
//                    recon_cv.at<uchar>(0,ii) = cv::saturate_cast<uchar>(recon_vigra(ii,index)*quant); // /(*(scaling_))(0,index));
                }
            }
//            for(int ii=0; ii<rows_; ii++)
            cv::Mat tmp = recon_cv.reshape(channels_, winSize_);
            cv::Mat region( outputImage,  cv::Rect(i,j,winSize_, winSize_) );
            tmp.copyTo(region);
            index++;
        }
    }

    cv::imwrite(fileName , outputImage);
}

bool Samples::loadImage(std::string& fileName, int winSize, int channels, int step) {
    cv::Mat inputImage;

    channels_ = channels;
    winSize_ = winSize;
    switch(channels_) {
//    case 1:
//       inputImage = cv::imread(fileName, 0);
//       break;
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
    cols_ = ceil((float)imageRows_/(float)step) * ceil((float)imageCols_/(float)step);
    std::cout << cols_ << " " << imageRows_ <<  " " << step <<std::endl;
    if(data_) delete data_;
    data_ = new MatrixXd(rows_, cols_);

    std::vector<cv::Mat> planes;
    split(inputImage, planes);

    int index = 0;
    for(int j=0; j<imageRows_; j+=step) {
        for(int i=0; i<imageCols_; i+=step) {

            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<double>(0,2)=-i;
            transMat.at<double>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(inputImage, warped, transMat, cv::Size(winSize_, winSize_));
            std::vector<cv::Mat> planes;
            split(warped, planes);
            for (int jj=0; jj<channels; jj++) {
                cv::Mat tmp = planes[jj].reshape(1,1);
                for(int ii=0; ii<rows_/channels_; ii++) {
                    (*data_)(jj*(rows_/channels_)+ii,index) = tmp.at<uchar>(0,ii);
                }
            }
//            (*data_).col(index).normalize();
            index++;
        }
    }
    // normalize the input
//    vigra::Matrix<double> offset(1,cols_);
//    scaling_ = new vigra::Matrix<double>(1,cols_);
//    prepareColumns((*data_), (*data_), offset, (*scaling_), vigra::linalg::DataPreparationGoals(vigra::linalg::UnitNorm));
    return true;
}
