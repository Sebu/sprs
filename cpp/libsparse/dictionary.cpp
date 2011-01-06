#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fstream>

#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

#include <eigen2/Eigen/Array>

Dictionary::Dictionary(int size, int channels, int elementCount) :data_(0), signalSize_(size*size*channels),
    elements_(elementCount), channels_(channels), blockSize_(size), meta_(0)
{
    clear();
    init(signalSize_, elements_);
}


void Dictionary::init(int signalSize, int elements) {
    elements_ = elements;
    signalSize_ = signalSize;
    data_ = new MatrixXd(signalSize_, elements_);
    meta_ = std::vector<MetaDict>(elements_);
}


void Dictionary::clear() {
   if(data_) { delete data_; data_=0;}
   meta_.clear();
//   if(meta_) { delete[] meta_; meta_=0; }
}

void Dictionary::save(const char* fileName) {
    std::ofstream ofs( fileName );

    ofs << DICT_VERSION << signalSize_ << " " << elements_ << " " << channels_ << " " << blockSize_ << " ";
    for(unsigned int i=0; i<elements_; i++) {
        ofs << meta_[i].usage_;
        for(unsigned int j=0; j<signalSize_; j++)
            ofs << (*data_)(j,i);
    }

    ofs.close();
}

void Dictionary::load(const char* fileName) {
    std::ifstream ifs( fileName );
    std::cout << "loading: " << fileName << std::endl;

    if (ifs) {
        ifs >> signalSize_;

        // VERSION CLASH :)
        if(signalSize_ == DICT_VERSION) { ifs >> signalSize_; std::cout << signalSize_ << std::endl; }
        ifs >>  elements_ >> channels_ >> blockSize_;
        clear();//        signalSize_ /= channels_;
        init(signalSize_, elements_);

        for(unsigned int i=0; i<elements_; i++) {
            ifs >> meta_[i].usage_;
            for(unsigned int j=0; j<signalSize_; j++)
                ifs >> (*data_)(j,i);
        }
    } else { std::cout << "dict not found" << std::endl; }

    ifs.close();
}


void Dictionary::centeR() {
        center((*data_));
}

void Dictionary::normalize() {
    for(int i=0; i<elements_; i++) {
        MatrixXd c = (*data_).col(i);
        c.normalize();
        (*data_).col(i) = c;
    }
}

void Dictionary::initRandom() {
    (*data_).setRandom();   
}

void Dictionary::initFromData(Samples& data) {
    srand ( time(NULL) );
    for(int j=0; j<this->elements_; j++) {
        int pos  = rand() % data.getData().cols();
        (*data_).col(j) = data.getData().col(pos);
    }
}


MatrixXd & Dictionary::getData() {
    return *data_;
}


void Dictionary::debugSaveImage(const char* filename) {

    int tmp = ceil(sqrt(elements_));

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
