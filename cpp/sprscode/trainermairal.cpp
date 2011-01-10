
#include "trainermairal.h"
#include "dictionary.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "samples.h"
#include "vigra_ext.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <eigen2/Eigen/Array>

TrainerMairal::TrainerMairal() : A_(0), B_(0)
{
}
void TrainerMairal::save(const char* fileName) {
    std::ofstream ofs( fileName );

    ofs << A_->rows() << " " <<  A_->cols() << " ";
    for(unsigned int i=0; i<A_->cols(); i++)
        for(unsigned int j=0; j<A_->rows(); j++)
            ofs << (*A_)(j,i) << " ";
    ofs << B_->rows() << " " <<  B_->cols() << " ";
    for(unsigned int i=0; i<B_->cols(); i++)
        for(unsigned int j=0; j<B_->rows(); j++)
            ofs << (*B_)(j,i) << " ";
    ofs.close();
}

void TrainerMairal::load(const char* fileName) {
    std::ifstream ifs( fileName );
    int rows=0, cols=0;
    if (ifs) {
        ifs >> rows >> cols;
        if(A_) delete A_;
        A_ = new MatrixXd(rows, cols);
        //        std::cout << rows << " " << cols << std::endl;
        for(unsigned int i=0; i<cols; i++)
            for(unsigned int j=0; j<rows; j++)
                ifs >> (*A_)(j,i);
        ifs >> rows >> cols;
        if(B_) delete B_;
        B_ = new MatrixXd(rows, cols);
        //        std::cout << rows << " " << cols << std::endl;
        for(unsigned int i=0; i<cols; i++)
            for(unsigned int j=0; j<rows; j++)
                ifs >> (*B_)(j,i);

    }

    ifs.close();
}

void TrainerMairal::update(MatrixXd& A, MatrixXd& B, Dictionary& D) {
    //    for(int i=0; i < 1; i++) {
#pragma omp parallel for
    for(int j=0; j < D.getElementCount(); j++) {
        MatrixXd a = A.col(j);
        MatrixXd b = B.col(j);
        MatrixXd d = D.getData().col(j);
        double pivot = A.coeff(j,j);
        if(pivot==0.0) continue;
        //        if(isinf(pivot))
        //            std::cout << "Sdsd" << std::endl;

        MatrixXd u = ( (1.0/pivot) * (b-(D.getData()*a)) ) + d;
        D.getData().col(j) = (1.0/std::max(u.norm(),1.0)) * u;
    }
    //}

}

void TrainerMairal::train(Samples& samples, Dictionary& D, int iterations, int batch) {

    std::cout << "train..." << std::endl;
    if(!A_ && !B_) {
        A_ = new MatrixXd(D.getElementCount(), D.getElementCount());
        B_ = new MatrixXd(D.getSignalSize(), D.getElementCount());
        // init A,B with 0
        A_->setZero();
        B_->setZero();
        //        std::cout << (*A_) << std::endl;
        std::cout << "init A &  B "<< std::endl;
    }
    std::cout << "train start" << std::endl;

    int maximum = samples.cols_;

    if (iterations) maximum = batch*iterations;

    D.normalize();
    center(D.getData());

    for(int t=0; t<maximum; t+=batch) {


        // draw sample from trainig set
        int start = t;
        int end = std::min(t+batch,samples.cols_);
        if (start>=end) break;
        std::cout << "samples: " <<  end << std::endl;

        // sparse code sample
        MatrixXd sample = samples.getData().block(0,start,D.getSignalSize(),end-start);
        center(sample);
        Eigen::SparseMatrix<double> a = coder->encode(sample, D);
        D.meta_->samples_+=sample.cols();
        std::cout << "a*a.transpose();" << std::endl;

//        MatrixXd aa = MatrixXd::Zero(a.rows(),a.cols());

        for (int k=0; k<a.outerSize(); ++k)
            for (Eigen::SparseMatrix<double>::InnerIterator it(a,k); it; ++it) {
              D.meta_->col_[it.row()].usage_++;
            }

        Eigen::SparseMatrix<double> tmp = a * a.transpose();
        std::cout << "(*A_) += tmp;" << std::endl;

        for (int k=0; k<tmp.outerSize(); ++k)
            for (Eigen::SparseMatrix<double>::InnerIterator it(tmp,k); it; ++it) {
                double value = it.value();
                (*A_)(it.row(),it.col()) += value;
                //                if((value!=value) || isinf(value))
                //                    std::cout << value << std::endl;
            }

        (*B_) += sample*a.transpose();
        // update step (algo. 2)
        std::cout << "update((*A_), (*B_), D);" << std::endl;
        update((*A_), (*B_), D);
        std::ostringstream o;
        o << "../../output/tmp/dict_tmp" << t << ".png";
        D.debugSaveImage( o.str().c_str() );

    }

}
