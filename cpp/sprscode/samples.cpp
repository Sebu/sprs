#include "samples.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "dictionary.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <fstream>
#include "vigra_ext.h"
#include "huffman.h"
#include "rle.h"
#include "sprscode.h"

Samples::Samples() : data_(0) //, scaling_(0)
{
}

MatrixXd & Samples::getData() {
    return *data_;
}


void Samples::normalize() {
    for(int i=0; i<(*data_).cols(); i++) {
        (*data_).col(i).normalize();
    }
}

void Samples::saveImage(std::string& fileName, Dictionary& dict, Coder& coder, int step) {

//    quant_ = 20.0;
    coeffs_ = coder.coeffs;
    Sprscode spc(imageCols_, imageRows_, channels_, blockSize_, coeffs_);
    spc.header_.quant_ = (int)round(quant_);

//    std::cout << "restore image" << dict.getData().rows() << " " <<  (*data_).rows() << std::endl;

    VectorXd shift = center((*data_));

    VectorXd scale((*data_).cols());
    for(int i=0; i<(*data_).cols(); i++) {
        scale(i) = 1.0;
        if((*data_).col(i).squaredNorm()!=0.0) {
            scale(i) = (*data_).col(i).norm();
            (*data_).col(i).normalize();
        }
    }

    Eigen::SparseMatrix<double> A = coder.encode((*data_), dict);

//    std::cout << A.nonZeros()/A.outerSize() << std::endl;

    // scale back
    VectorXd select = VectorXd::Zero(A.innerSize());
    for (int k=0; k<A.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {
        A.coeffRef(it.row(),it.col()) *= scale(it.col());
        if(it.col()==200) select(it.row()) = A.coeffRef(it.row(),it.col());
        }
    }
    for(int i=0; i<(*data_).cols(); i++) {
       (*data_).col(i)*= scale(i);
    }

    spc.compress(shift,A);
    spc.save(fileName);

    //reconstruct :)
//    std::cout << "uncopressssssssssssssssssssssssssss" << std::endl;
    //TODO: fill a new A
    spc.load(fileName);
    spc.uncompress(shift,A);


    MatrixXd recon_vigra = dict.getData()*A;

//    std::cout << A << std::endl;

//    for(int i=0; i<recon_vigra.cols(); i++) {
//        recon_vigra.col(i) *= scale(i);
//        (*data_).col(i)*= scale(i);
//    }


//    for (int k=0; k<A.outerSize(); ++k) {
//        for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {
//            A.coeffRef(it.row(),it.col()) = it.value()*scale(it.row());
//        }
//    }

    unshift((*data_),shift);
    unshift(recon_vigra,shift);

    double mseVal = mse((*data_),recon_vigra);
    std::cout << dict.meta_->samples_ << " " << psnr(mseVal) << std::endl;
//    std::cout << "PSNR: " << psnr(mseVal) << " dB" << std::endl;
//    std::cout << "MSE: " << mseVal << std::endl;


    dict.debugSaveImage("../../output/tmp/dict_tmp.select.png", select);


//    std::cout << "reorder  image" << std::endl;

    cv::Mat outputImage = cv::Mat::zeros(imageRows_+blockSize_, imageCols_+blockSize_, CV_8UC(channels_));
    cv::Mat recon_cv(1,rows_,CV_8U);
    int index = 0;
    for(int j=0; j<imageRows_; j+=step) {
        for(int i=0; i<imageCols_; i+=step) {

            for(int jj=0; jj<channels_; jj++){
                for(int ii=0; ii<rows_/channels_; ii++) {
                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>(recon_vigra(jj*(rows_/channels_)+ii, index));
                    //                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>(recon_vigra(ii,index*channels_+jj));
                }
            }

            cv::Mat tmp = recon_cv.reshape(channels_, blockSize_);
            //            cv::Mat tmp = inshape(recon_cv, winSize_,  channels_);

            cv::Mat region( outputImage,  cv::Rect(i,j,blockSize_, blockSize_) );
            region += tmp;
//            region = region/2.0 + tmp/2.0;
            //tmp.copyTo(region);
            index++;
        }
    }
    cv::Mat im(outputImage, cv::Rect(0,0,imageCols_, imageRows_));
//    cv::cvtColor(im, im, CV_YCrCb2RGB);

    cv::imshow("sprscode", im);
    cv::waitKey();
    cv::imwrite(fileName, im);
}

bool Samples::loadImage(std::string& fileName, int winSize, int channels, int step) {
    cv::Mat inputImage;
    channels_ = channels;
    blockSize_ = winSize;
    switch(channels_) {
    case 0:
        inputImage = cv::imread(fileName,-1);
        channels_ = inputImage.channels();
        break;
    case 1:
        inputImage = cv::imread(fileName, 0);
        break;
    case 3:
        inputImage = cv::imread(fileName);
        break;

    }

    if( inputImage.data==NULL ) {
        std::cout << "can't read image" << std::endl;
        return false;
    }
    //    cv::resize(inputImage, tmpMat, cv::Size(256,256));
    //    inputImage = tmpMat;
    cv::Mat lowPass =  inputImage.clone();
    cv::Mat highPass =  inputImage.clone();
    cv::blur(lowPass,lowPass,cv::Size(1,4));
//    inputImage= lowPass - highPass;
//    cv::imwrite("/tmp/debug1.png",inputImage);
//    cv::imwrite("/tmp/debug2.png",lowPass);

    return sampleImage(inputImage, step);


//    cv::cvtColor(inputImage, inputImage, CV_RGB2YCrCb);
}


bool Samples::sampleImage(cv::Mat inputImage, int step) {



    imageRows_ = inputImage.rows;
    imageCols_ = inputImage.cols;
    rows_ = blockSize_*blockSize_*channels_;
    cols_ = ceil((float)imageRows_/(float)step) * ceil((float)imageCols_/(float)step);

    //    cols_*=channels_;
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
            cv::warpAffine(inputImage, warped, transMat, cv::Size(blockSize_, blockSize_));
            std::vector<cv::Mat> planes;
            split(warped, planes);
            for (int jj=0; jj<channels_; jj++) {
                cv::Mat tmp = planes[jj].reshape(1,1);
                //                cv::Mat tmp = unshape(planes[jj],winSize_,1);
                for(int ii=0; ii<rows_/channels_; ii++) {
                    (*data_)(jj*(rows_/channels_)+ii,index) = tmp.at<uchar>(0,ii);
                    //                   (*data_)(ii,index*channels_+jj) = tmp.at<uchar>(0,ii);
                }
            }
            index++;
        }
    }
    return true;
}
