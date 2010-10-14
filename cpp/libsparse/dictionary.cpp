#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>

#include "dictionary.h"

Dictionary::Dictionary(int size, int channels, int elementCount) :data_(0), signalSize_(size*size*channels),
    elementCount_(elementCount), channels_(channels), size_(size)
{
    data_ = new vigra::Matrix<double>(signalSize_, elementCount_);
}


void Dictionary::save(char *fileName) {
    std::ofstream ofs( fileName );

    ofs << signalSize_ << " " << elementCount_ << " " << channels_ << " " << size_ << " ";
    for(unsigned int i=0; i<elementCount_; i++)
        for(unsigned int j=0; j<signalSize_; j++)
            ofs << (*data_)(j,i);
    ofs.close();
}

void Dictionary::load(char *fileName) {
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




const vigra::Matrix<double> & Dictionary::getData() {
    return *data_;
}

void Dictionary::update(Matrix<double>& A, Matrix<double>& B) {

    for(int i=0; i < 1; i++) {
        for(int j=0; j < elementCount_; j++) {
            Matrix<double> a = A.columnVector(j);
            Matrix<double> b = B.columnVector(j);
            Matrix<double> d = data_->columnVector(j);
            //            std::cout << A(j,j) << std::endl;
            if(A(j,j)==0.0) continue;
            Matrix<double> u = ( (1.0/A(j,j)) * (b-((*data_)*a)) ) + d;
            Matrix<double> tmp = (1.0/fmax(u.norm(),1.0)) * u;
            for(int i=0; i< signalSize_; i++)
                (*data_)(i,j) = tmp(i,0);
        }
    }

}

void Dictionary::learn(Matrix<double>& samples, int iterations) {

    Matrix<double> A(elementCount_, elementCount_), B(signalSize_, elementCount_);

    // init A,B with 0
    A.init(0.0); B.init(0.0);

    for(int t=0; t<iterations; t++) {

        // draw sample from trainig set
        Matrix<double> sample = samples.columnVector(t);

        // sparse code sample
        Matrix<double> a = lasso(sample,*data_);

        std::cout << "Iteration: " <<  t << std::endl;

        A = A + mmul(a,a.transpose());
        B = B + mmul(sample,a.transpose());
        // update step (algo. 2)
        update(A,B);
    }

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
