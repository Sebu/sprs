#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>

#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

Dictionary::Dictionary(int size, int channels, int elementCount) :data_(0), signalSize_(size*size*channels),
    elementCount_(elementCount), channels_(channels), size_(size)
{
    data_ = new vigra::Matrix<double>(signalSize_, elementCount_);
}


void Dictionary::save(const char* fileName) {
    std::ofstream ofs( fileName );

    ofs << signalSize_ << " " << elementCount_ << " " << channels_ << " " << size_ << " ";
    for(unsigned int i=0; i<elementCount_; i++)
        for(unsigned int j=0; j<signalSize_; j++)
            ofs << (*data_)(j,i);
    ofs.close();
}

void Dictionary::load(const char* fileName) {
    std::ifstream ifs( fileName );

    if (ifs) {
        ifs >> signalSize_ >>  elementCount_ >> channels_ >> size_;
        for(unsigned int i=0; i<elementCount_; i++)
            for(unsigned int j=0; j<signalSize_; j++)
                ifs >> (*data_)(j,i);
    }

    ifs.close();
}




void Dictionary::initRandom() {
    init_random((*data_));
    prepareColumns((*data_), (*data_), DataPreparationGoals(UnitNorm));
}

void Dictionary::initFromData(Matrix<double> & data) {
    srand ( time(NULL) );
    for(int j=0; j<data_->size(1); j++) {
        int pos  = rand() % data.columnCount();
        std::cout << pos  << std::endl;
        for(int i=0; i<data_->size(0); i++) {
            (*data_)(i,j) = data(i,pos);
        }
    }
}




vigra::Matrix<double> & Dictionary::getData() {
    return *data_;
}


void Dictionary::debugSaveImage(char *filename) {

    int tmp = ceil(sqrt(elementCount_));

    vigra::Matrix<double> D(signalSize_, elementCount_);
    prepareColumns((*data_), D, DataPreparationGoals(UnitNorm));


    cv::Mat outputImage(size_*tmp, size_*tmp, CV_8UC(channels_));
    for(int j=0; j<size_*tmp; j+=size_) {
        for(int i=0; i<size_*tmp; i+=size_) {
            int index = ceil(j/size_)*tmp + ceil(i/size_);
            Matrix<double> d = D.columnVector(index);
            cv::Mat recon_cv(1, signalSize_, CV_8U);
            for(int ii=0; ii<signalSize_; ii++) {
                recon_cv.at<uchar>(0,ii) = (uchar)((d(ii,0)+1.0)*128.0);
            }
            cv::Mat tmp = recon_cv.reshape(channels_, size_);
            cv::Mat region( outputImage,  cv::Rect(i,j,size_,size_) );
            tmp.copyTo(region);
        }
    }
    cv::imwrite(filename, outputImage);

}
