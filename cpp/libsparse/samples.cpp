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

void Samples::saveImage(std::string& fileName, Dictionary& dict, Coder& coder) {

    double quant = 10.0;

    std::cout << "restore image" << std::endl;
    VectorXd shift = center((*data_));
    Eigen::SparseMatrix<double> A = coder.encode((*data_), dict);

    std::ofstream ofs( (fileName + ".sp").c_str(), std::ios::out | std::ios::binary );
    for (int k=0; k<A.outerSize(); ++k) {
      int count=0;
      for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {

          short data=0;
          data = (short)round(it.value()/quant);
          unsigned short pos=0;
          pos = (short)round(it.row());
          A.coeffRef(it.row(),it.col()) = data*quant;
          if(data) {
              ofs.write((char*)&pos,sizeof(pos));
              ofs.write((char*)&data,sizeof(data));
              count++;
//              std::cout << data << " ";
          }
      }
//      std::cout << count << " ";
    }
    std::cout << std::endl;
    ofs.close();
    unshift((*data_),shift);

//    MatrixXd recon_vigra = (*data_);
    MatrixXd recon_vigra = dict.getData()*A;

    unshift(recon_vigra,shift);

    std::cout << "PSNR: " << psnr((*data_),recon_vigra) << std::endl;
    std::cout << "RMSE: " << std::sqrt(mse((*data_),recon_vigra)) << std::endl;

    std::cout << "reorder  image" << std::endl;

    cv::Mat outputImage(imageRows_+winSize_, imageCols_+winSize_, CV_8UC(channels_));
    cv::Mat recon_cv(1,rows_,CV_8U);
    int index = 0;
    for(int j=0; j<imageRows_; j+=winSize_) {
        for(int i=0; i<imageCols_; i+=winSize_) {

            for(int jj=0; jj<channels_; jj++){
                for(int ii=0; ii<rows_/channels_; ii++) {
                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>(recon_vigra(jj*(rows_/channels_)+ii, index));
                }
            }

            cv::Mat tmp = recon_cv.reshape(channels_, winSize_);
//            cv::Mat tmp = inshape(recon_cv, winSize_,  channels_);

            cv::Mat region( outputImage,  cv::Rect(i,j,winSize_, winSize_) );
            tmp.copyTo(region);
            index++;
        }
    }
    cv::Mat im(outputImage, cv::Rect(0,0,imageCols_, imageRows_));
    cv::imwrite(fileName, im);
}

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
//    cv::Mat blub;
//    cv::resize(inputImage, blub, cv::Size(256,256));
//    inputImage = blub;
//    cv::imwrite("/Users/sebastian/Bilder/bla.png",inputImage);
    imageRows_ = inputImage.rows;
    imageCols_ = inputImage.cols;
    rows_ = winSize_*winSize_*channels_;
    cols_ = ceil((float)imageRows_/(float)step) * ceil((float)imageCols_/(float)step);
    //std::cout << cols_ << " " << imageRows_ <<  " " << step <<std::endl;
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
//                cv::Mat tmp = unshape(planes[jj],winSize_,1);
                for(int ii=0; ii<rows_/channels_; ii++) {
                    (*data_)(jj*(rows_/channels_)+ii,index) = tmp.at<uchar>(0,ii);
                }
            }
            index++;
        }
    }
    return true;
}
