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
    meta_ = new MetaDict();
    for(int i=0; i<elements_; ++i)
        meta_->col_.push_back(MetaUsage(i));

}


void Dictionary::clear() {
   if(data_) { delete data_; data_=0;}
   if (meta_) {
       meta_->col_.clear();
       delete meta_;
       meta_=0;
   }
}


void Dictionary::merge(Dictionary& input) {
    static int bla = 0;

    if(bla+300>=(*data_).cols()) return;

    for(int i=0; i<300; ++i) {
        std::cout << i << std::endl;
        (*data_).col(i+bla) = input.getData().col(i);
    }
    bla+=300;

}

void Dictionary::save(const char* fileName) {
    std::ofstream ofs( fileName );


    ofs << DICT_VERSION << " " <<  signalSize_ << " " << elements_ << " " << channels_ << " " << blockSize_ << " ";
    std::cout << DICT_VERSION << " " << signalSize_ << " " << elements_ << " " << channels_ << " " << blockSize_ << " ";
    for(unsigned int i=0; i<elements_; i++) {
        for(unsigned int j=0; j<signalSize_; j++) {
            ofs << (*data_)(j,i) << " ";
        }
    }
    ofs.close();

    std::string metaFile = std::string(fileName) + ".meta";
    std::ofstream ofsM( metaFile.c_str() );
    ofsM << DICT_VERSION << " " << meta_->samples_ << " ";
    for(unsigned int i=0; i<elements_; i++)
        ofsM << meta_->col_[i].usage_ << " ";
    ofsM.close();
}

void Dictionary::load(const char* fileName) {
    std::ifstream ifs( fileName );
    std::cout << "loading: " << fileName << std::endl;

    if (ifs) {
        int version;
        ifs >> version;
        ifs >> signalSize_ >> elements_ >> channels_ >> blockSize_;
        clear();
        std::cout << signalSize_ << " " << elements_ << " " << channels_ << " " << blockSize_ << " ";
        init(signalSize_, elements_);

        for(unsigned int i=0; i<elements_; i++) {
            for(unsigned int j=0; j<signalSize_; j++) {
                ifs >> (*data_)(j,i);
            }
        }
    } else { std::cout << "dict not found" << std::endl; }
    ifs.close();


    std::string metaFile = std::string(fileName) + ".meta";
    std::ifstream ifsM( metaFile.c_str() );
    if (ifsM) {
        int version;
        ifsM >> version >> meta_->samples_;
        for(unsigned int i=0; i<elements_; i++)
          ifsM >> meta_->col_[i].usage_;
    } else { meta_->rewrite_=true; std::cout << "no meta data :/" << std::endl; }
    ifsM.close();
    // if (meta_->rewrite_) save(fileName);
}

bool colSorterBigFirst(const MetaUsage& i, const MetaUsage& j) { return ( i.usage_ > j.usage_ ); }

void Dictionary::sort() {
     std::sort(meta_->col_.begin(), meta_->col_.end(), colSorterBigFirst);
     MatrixXd tmp = (*data_);
     for(int i=0; i<tmp.cols(); ++i) {
         (*data_).col(i) = tmp.col(meta_->col_[i].id_);
     }
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