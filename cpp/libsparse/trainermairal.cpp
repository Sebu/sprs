
#include "trainermairal.h"
#include "dictionary.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "samples.h"

#include <algorithm>
#include <iostream>
#include <fstream>

typedef vigra::MultiArray<2, double>::difference_type Shape;

TrainerMairal::TrainerMairal() : A_(0), B_(0)
{
}
void TrainerMairal::save(const char* fileName) {
    std::ofstream ofs( fileName );

//    ofs << A_->rows() << " " <<  A_->cols() << " ";
//    for(unsigned int i=0; i<A_->cols(); i++)
//        for(unsigned int j=0; j<A_->rows(); j++)
//            ofs << (*A_)(j,i) << " ";
//    ofs << B_->rows() << " " <<  B_->cols() << " ";
//    for(unsigned int i=0; i<B_->cols(); i++)
//        for(unsigned int j=0; j<B_->rows(); j++)
//            ofs << (*B_)(j,i) << " ";
    ofs.close();
}

void TrainerMairal::load(const char* fileName) {
    std::ifstream ifs( fileName );
    int rows=0, cols=0;
//    if (ifs) {
//        ifs >> rows >> cols;
//        if(A_) delete A_;
//        A_ = new MatrixXf(rows, cols);
////        std::cout << rows << " " << cols << std::endl;
//        for(unsigned int i=0; i<cols; i++)
//            for(unsigned int j=0; j<rows; j++)
//                ifs >> (*A_)(j,i);
//        ifs >> rows >> cols;
//        if(B_) delete B_;
//        B_ = new MatrixXf(rows, cols);
////        std::cout << rows << " " << cols << std::endl;
//        for(unsigned int i=0; i<cols; i++)
//            for(unsigned int j=0; j<rows; j++)
//                ifs >> (*B_)(j,i);

//    }

    ifs.close();
}

void TrainerMairal::update(MatrixXf& A, MatrixXf& B, Dictionary& D) {

    for(int i=0; i < 1; i++) {
        for(int j=0; j < D.getElementCount(); j++) {
            MatrixXf a = A.col(j);
            MatrixXf b = B.col(j);
            MatrixXf d = D.getData().col(j);
            if(A.coeff(j,j)==0.0) continue;
            MatrixXf u = ( (1.0/A.coeff(j,j)) * (b-(D.getData()*a)) ) + d;
            D.getData().col(j) = (1.0/fmax(u.norm(),1.0)) * u;
        }
    }

}

void TrainerMairal::train(Samples& samples, Dictionary& D, int iterations, int batch) {

    CoderOMP coder;

    std::cout << "train..." << std::endl;
    if(!A_ && !B_) {
        A_ = new MatrixXf(D.getElementCount(), D.getElementCount());
        B_ = new MatrixXf(D.getSignalSize(), D.getElementCount());
        // init A,B with 0
        A_->setZero();
        B_->setZero();
//        std::cout << (*A_) << std::endl;
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
        std::cout << "samples: " <<  end << std::endl;

        // sparse code sample
        MatrixXf sample = samples.getData().block(0,start,D.getSignalSize(),end-start);
        Eigen::SparseMatrix<float> a = coder.encode(sample, D);

//        std::cout << "a*a.transpose();" << std::endl;
        Eigen::SparseMatrix<float> tmp = a*a.transpose();
//        std::cout << "(*A_) += tmp;" << std::endl;
        for (int k=0; k<tmp.outerSize(); ++k)
          for (Eigen::SparseMatrix<float>::InnerIterator it(tmp,k); it; ++it) {
              (*A_)(it.row(),it.col()) += it.value();
          }

        (*B_) += sample*a.transpose();
        // update step (algo. 2)
//        std::cout << "update((*A_), (*B_), D);" << std::endl;
        update((*A_), (*B_), D);
        D.debugSaveImage( "/tmp/dict_tmp.png" );

    }

}
