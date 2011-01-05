#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>

#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

#include <eigen2/Eigen/Array>

Dictionary::Dictionary(int size, int channels, int elementCount) :data_(0), signalSize_(size*size*channels),
    elementCount_(elementCount), channels_(channels), blockSize_(size)
{
    data_ = new MatrixXd(signalSize_, elementCount_);
}


void Dictionary::save(const char* fileName) {
    std::ofstream ofs( fileName );

    ofs << signalSize_ << " " << elementCount_ << " " << channels_ << " " << blockSize_ << " ";
    for(unsigned int i=0; i<elementCount_; i++)
        for(unsigned int j=0; j<signalSize_; j++)
            ofs << (*data_)(j,i);
    ofs.close();
}

void Dictionary::load(const char* fileName) {
    std::ifstream ifs( fileName );
    std::cout << "loading: " << fileName << std::endl;

    if (ifs) {
        ifs >> signalSize_ >>  elementCount_ >> channels_ >> blockSize_;
        if(data_) delete data_;
//        signalSize_ /= channels_;
        data_ = new MatrixXd(signalSize_, elementCount_);
        for(unsigned int i=0; i<elementCount_; i++)
            for(unsigned int j=0; j<signalSize_; j++)
                ifs >> (*data_)(j,i);
    }

    ifs.close();
}


void Dictionary::centeR() {
        center((*data_));
}

void Dictionary::normalize() {
    for(int i=0; i<elementCount_; i++) {
        MatrixXd c = (*data_).col(i);
        c.normalize();
//        center(c);
        (*data_).col(i) = c;
    }
}

void Dictionary::initRandom() {
    (*data_).setRandom();   
//    normalize();
}

void Dictionary::initFromData(Samples& data) {
    srand ( time(NULL) );
    for(int j=0; j<this->elementCount_; j++) {
        int pos  = rand() % data.getData().cols();
//        std::cout << (*data_).rows() << " " << data.getData().rows() << std::endl;
        (*data_).col(j) = data.getData().col(pos);
//        for(int i=0; i<data.getData().rows(); i++) {
//            (*data_)(i,j) = data.getData()(i,pos);
//        }
    }
}




MatrixXd & Dictionary::getData() {
    return *data_;
}


void Dictionary::debugSaveImage(const char* filename) {

    int tmp = ceil(sqrt(elementCount_));

    MatrixXd D = (*data_);

    cv::Mat outputImage(blockSize_*tmp, blockSize_*tmp, CV_8UC(channels_));
//    cv::Mat outputImage(blockSize_*tmp, blockSize_*tmp, CV_8U);
    for(int j=0; j<blockSize_*tmp; j+=blockSize_) {
        for(int i=0; i<blockSize_*tmp; i+=blockSize_) {
            int index = ceil(j/blockSize_)*tmp + ceil(i/blockSize_);
            if(index>=D.cols()) break;
            MatrixXd d = D.col(index);
            cv::Mat recon_cv(1, signalSize_, CV_8U);

            double min=DBL_MAX, max=DBL_MIN;
            for(int ii=0; ii<signalSize_; ii++) {
                double val = d(ii,0);
                 if(val<min) min = val;
                 if(val>max) max = val;
            }
            double scale = 255.0/(max - min);

//                for(int ii=0; ii<signalSize_; ii++) {
//                    recon_cv.at<uchar>(0,ii) = cv::saturate_cast<uchar>((d(ii,0)-min)*scale);
//                }
            for(int jj=0; jj<channels_; jj++){
                for(int ii=0; ii<signalSize_/channels_; ii++) {
                    recon_cv.at<uchar>(0,ii*channels_+jj) = cv::saturate_cast<uchar>((d(jj*(signalSize_/channels_)+ii,0)-min)*scale);
                }
            }
            cv::Mat tmp = recon_cv.reshape(channels_, blockSize_);
//            cv::Mat tmp = inshape(recon_cv, size_,  channels_);
            cv::Mat region( outputImage,  cv::Rect(i,j,blockSize_,blockSize_) );
            tmp.copyTo(region);
        }
    }
    cv::imwrite(filename, outputImage);

}
