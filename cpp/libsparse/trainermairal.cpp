
#include "trainermairal.h"
#include "dictionary.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "samples.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include <vigra/multi_array.hxx>
typedef vigra::MultiArray<2, double>::difference_type Shape;

TrainerMairal::TrainerMairal() : A_(0), B_(0)
{
}
void TrainerMairal::save(const char* fileName) {
    std::ofstream ofs( fileName );

    ofs << A_->rowCount() << " " <<  A_->columnCount() << " ";
    for(unsigned int i=0; i<A_->columnCount(); i++)
        for(unsigned int j=0; j<A_->rowCount(); j++)
            ofs << (*A_)(j,i) << " ";
    ofs << B_->rowCount() << " " <<  B_->columnCount() << " ";
    for(unsigned int i=0; i<B_->columnCount(); i++)
        for(unsigned int j=0; j<B_->rowCount(); j++)
            ofs << (*B_)(j,i) << " ";
    ofs.close();
}

void TrainerMairal::load(const char* fileName) {
    std::ifstream ifs( fileName );
    int rows=0, cols=0;
    if (ifs) {
        ifs >> rows >> cols;
        if(A_) delete A_;
        A_ = new vigra::Matrix<double>(rows, cols);
//        std::cout << rows << " " << cols << std::endl;
        for(unsigned int i=0; i<cols; i++)
            for(unsigned int j=0; j<rows; j++)
                ifs >> (*A_)(j,i);
        ifs >> rows >> cols;
        if(B_) delete B_;
        B_ = new vigra::Matrix<double>(rows, cols);
//        std::cout << rows << " " << cols << std::endl;
        for(unsigned int i=0; i<cols; i++)
            for(unsigned int j=0; j<rows; j++)
                ifs >> (*B_)(j,i);

    }

    ifs.close();
}

void TrainerMairal::update(vigra::Matrix<double>& A, vigra::Matrix<double>& B, Dictionary& D) {

    for(int i=0; i < 1; i++) {
        for(int j=0; j < D.getElementCount(); j++) {
            vigra::Matrix<double> a = A.columnVector(j);
            vigra::Matrix<double> b = B.columnVector(j);
            vigra::Matrix<double> d = D.getData().columnVector(j);
            if(A(j,j)==0.0) continue;
            vigra::Matrix<double> u = ( (1.0/A(j,j)) * (b-(D.getData()*a)) ) + d;
            vigra::Matrix<double> tmp = (1.0/fmax(u.norm(),1.0)) * u;
            for(int i=0; i< D.getSignalSize(); i++)
                D.getData()(i,j) = tmp(i,0);
        }
    }

}

void TrainerMairal::train(Samples& samples, Dictionary& D, int iterations, int batch) {

    CoderOMP coder;

    std::cout << "train..." << std::endl;
    if(!A_ && !B_) {
        A_ = new vigra::Matrix<double>(D.getElementCount(), D.getElementCount());
        B_ = new vigra::Matrix<double>(D.getSignalSize(), D.getElementCount());
        // init A,B with 0
        A_->init(0.0); B_->init(0.0);
//        std::cout << "init A &  B "<< std::endl;
    }
    std::cout << "train start" << std::endl;

    int maximum = samples.cols_;
    if (iterations) maximum = batch*iterations;
    for(int t=0; t<maximum; t+=batch) {

        // draw sample from trainig set
        int start = t;
        int end = std::min(t+batch,samples.cols_);
        if (start>=end) break;
        std::cout << "samples: " <<  end-start << std::endl;
        vigra::Matrix<double> sample = samples.getData().subarray(Shape(0,start), Shape(D.getSignalSize(),end));

        // sparse code sample
        vigra::Matrix<double> a = coder.encode(sample, D);


        (*A_) = (*A_) + mmul(a,a.transpose());
        (*B_) = (*B_) + mmul(sample,a.transpose());
        // update step (algo. 2)
        update((*A_), (*B_), D);
    }

}
