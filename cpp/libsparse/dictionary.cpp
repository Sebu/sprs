#include "dictionary.h"

Dictionary::Dictionary(int size, int elements) : signalSize_(size), elementCount_(elements)
{
    data_ = new vigra::Matrix<double>(signalSize_, elementCount_);
    initRandom();
}

void Dictionary::initRandom() {
    init_random((*data_));
    prepareColumns((*data_), (*data_), DataPreparationGoals(ZeroMean|UnitVariance));
}

vigra::Matrix<double> & Dictionary::getData() {
    return *data_;
}

void Dictionary::update(Matrix<double>& A, Matrix<double>& B) {

    for(int i=0; i < 1; i++) {
        for(int j=0; j < elementCount_; j++) {
            Matrix<double> a = A.columnVector(j);
            if(A(j,j)==0.0) continue;
            Matrix<double> b = B.columnVector(j);
            Matrix<double> d = data_->columnVector(j);
            Matrix<double> u = ( (1.0/A(j,j)) * (b-((*data_)*a)) ) + d;
            Matrix<double> tmp = (1.0/fmax(u.norm(),0.0)) * u;
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
