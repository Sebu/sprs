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

Samples::Samples() : data_(0) //, scaling_(0)
{
}

MatrixXd & Samples::getData() {
    return *data_;
}

void Samples::saveImage(std::string& fileName, Dictionary& dict, Coder& coder) {

    quant_ = 40.0;


    std::cout << "restore image" << dict.getData().rows() << " " <<  (*data_).rows() << std::endl;

    VectorXd shift = center((*data_));
    Eigen::SparseMatrix<double> A = coder.encode((*data_), dict);
    unshift((*data_),shift);

    // std::ofstream ofs( (fileName + ".sp").c_str(), std::ios::out | std::ios::binary );

    FILE * pFile;
    pFile = fopen((fileName + ".sp").c_str(),"w");

    std::cout << A.cols() << std::endl;

    int sizeMax = sizeof(short)*(2*A.nonZeros()+shift.size());
//    int sizeMax = sizeof(short)*((A.rows()*A.cols())+shift.size());
    unsigned short* inBuf = (unsigned short*)malloc(sizeMax);
    unsigned short* outBuf = (unsigned short*)malloc(sizeMax);


    long count=0;
    for (int k=0; k<shift.size(); ++k) {
        short shiftVal = (short)round(shift(k));
        inBuf[count++] =  shiftVal;
        shift(k) = shiftVal;
    }
        double max=FLT_MIN;
//    for (long k=0; k<A.cols(); ++k) {
//        for (long j=0; j<A.rows(); ++j)  {
                for (int k=0; k<A.outerSize(); ++k) {
                    for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {

            short data=0;
//            data =(short)round(A.coeff(j,k)/quant_);
            data = (short)round(it.value()/quant_);
            unsigned short pos=0;
            pos = (unsigned short)it.row();
            A.coeffRef(it.row(),it.col()) = data*quant_;
            //if(data) {
            if(!data) pos = 0;
             inBuf[count] = pos;
            inBuf[count+A.nonZeros()] = data;
            if(data>max)max=data;
//            inBuf[count]=data;
            count++;
//            std::cout << count << " " << sizeMax << std::endl;

            //}
        }

    }
                std::cout << "max: " << max << std::endl;
    std::cout << (sizeof(short)*(count+A.nonZeros()))-sizeMax << std::endl;
    int inSize = sizeMax; //sizeof(short)*count;
    int outSize = Huffman_Compress((unsigned char*)inBuf,(unsigned char*)outBuf,inSize);
//    outSize = Huffman_Compress((unsigned char*)outBuf,(unsigned char*)inBuf,outSize);
    std::cout << "Index compression " << (float)inSize/(float)outSize << std::endl;

    fwrite ((char*)inBuf , outSize, sizeof(char), pFile );
    fclose(pFile);



    //    MatrixXd recon_vigra = (*data_);
    MatrixXd recon_vigra = dict.getData()*A;

    unshift(recon_vigra,shift);

    double mseVal = mse((*data_),recon_vigra);
    std::cout << "PSNR: " << psnr(mseVal) << " dB" << std::endl;
    std::cout << "RMSE: " << std::sqrt(mseVal) << std::endl;
    std::cout << "BPP: " <<  ((float)outSize*8.0)/(float)(imageRows_*imageCols_) << std::endl;
    std::cout << "compression: " <<  (float)(imageRows_*imageCols_*3)/((float)outSize) << ":1" << std::endl;

    std::cout << "reorder  image" << std::endl;

    cv::Mat outputImage(imageRows_+blockSize_, imageCols_+blockSize_, CV_8UC(channels_));
    cv::Mat recon_cv(1,rows_,CV_8U);
    int index = 0;
    for(int j=0; j<imageRows_; j+=blockSize_) {
        for(int i=0; i<imageCols_; i+=blockSize_) {

            for(int jj=0; jj<channels_; jj++){
                for(int ii=0; ii<rows_/channels_; ii++) {
                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>(recon_vigra(jj*(rows_/channels_)+ii, index));
                    //                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>(recon_vigra(ii,index*channels_+jj));
                }
            }

            cv::Mat tmp = recon_cv.reshape(channels_, blockSize_);
            //            cv::Mat tmp = inshape(recon_cv, winSize_,  channels_);

            cv::Mat region( outputImage,  cv::Rect(i,j,blockSize_, blockSize_) );
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
    blockSize_ = winSize;
    switch(channels_) {
    case 0:
        inputImage = cv::imread(fileName,-1);
        std::cout << inputImage.channels() << std::endl;
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
    //    cv::Mat tmpMat;
    //    cv::resize(inputImage, tmpMat, cv::Size(256,256));
    //    inputImage = tmpMat;
    //    cv::imwrite("/tmp/debug.png",inputImage);
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
