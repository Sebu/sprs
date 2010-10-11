#include "dictionary.h"

Dictionary::Dictionary(int size, int elements) : signalSize_(size), elementCount_(elements)
{
    data_ = new vigra::Matrix<double>(signalSize_, elementCount_);
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

    //    int dim = sqrt(elementCount_);
    //    prepareColumns(*data_, *data_, DataPreparationGoals(UnitNorm));

    //    cv::Mat outputImage(8*dim,8*dim,CV_8UC3);

    //    for(int j=0; j<8*dim; j+=size) {
    //        for(int i=0; i<8*dim; i+=size) {
    //            index = ceil(j/8)*ceil((8*dim)/8) + ceil(i/8);
    //            Matrix<double> d = D.columnVector(index);
    //            cv::Mat recon_cv(1,m,CV_8U);
    //            for(int ii=0; ii<m; ii++)
    //                recon_cv.at<uchar>(0,ii) = (uchar)(d(ii,0)*256.0f);
    //            cv::Mat tmp = recon_cv.reshape(3, size);
    //            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
    //            tmp.copyTo(region);
    //        }
    //    }
    //    cv::imwrite("/homes/wheel/seb/Bilder/dict_lasso.jpg", outputImage);

}
