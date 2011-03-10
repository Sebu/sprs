
#include "trainermairal.h"
#include "dictionary.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "samples.h"
#include "vigra_ext.h"
#include <algorithm>
#include <iostream>
#include <fstream>
//#include <Eigen/Array>

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
//            std::cout<< u.norm() << " ";
        }
//    }

}

void TrainerMairal::train(Samples& samples, Dictionary& D, int iterations, int batch) {


    //    std::cout << "train..." << std::endl;
    if(!A_ && !B_) {
        center(D.getData());
        D.normalize();
//        divVariance(D.getData());
        A_ = new MatrixXd(0.0001*MatrixXd::Identity(D.getElementCount(), D.getElementCount()) );
        B_ = new MatrixXd(0.0001*D.getData()); //new MatrixXd(D.getSignalSize(), D.getElementCount());
        //    std::cout << "train start" << std::endl;
    }

    int maximum = samples.cols_;

    if (iterations) maximum = batch*iterations;

    static double realT = 0.0;

    for(int t=0; t<maximum; t+=batch) {

        D.normalize();
        // draw sample from trainig set
        int start = t;
        int end = std::min(t+batch,samples.cols_);
        if (start>=end) break;
        //        std::cout << "samples: " <<  end << std::endl;

        // sparse code sample
        MatrixXd samplesChunk = samples.getData().block(0,start,D.getSignalSize(),end-start);

        center(samplesChunk);
//        divVariance(samplesChunk);
                for(int i=0; i<samplesChunk.cols(); i++) {
                    if(samplesChunk.col(i).norm()!=0.0) {
                        samplesChunk.col(i).normalize();
                    }
                }

        //        std::cout << samplesChunk << std::endl;
//                divVariance(samplesChunk);


        Eigen::SparseMatrix<double> a = coder->encode(samplesChunk, D);
        D.meta_->samples_+=samplesChunk.cols();
        //        std::cout << "| a*a.t() ";// << std::endl;


        //        MatrixXd aa = MatrixXd::Zero(a.rows(),a.cols());

        for (int k=0; k<a.outerSize(); ++k)
            for (Eigen::SparseMatrix<double>::InnerIterator it(a,k); it; ++it) {
                D.meta_->col_[it.row()].usage_++;
                //              std::cout << it.value() << std::endl;

            }
        //        double delta = t;
        //        double realT = t/batch + 1.0;
        //        double n = batch;
        //        if(realT<n)
        //            delta = realT*n;
        //        else
        //            delta = n*n+realT-n;
        realT += 1.0;
        double beta = 1.0; //(1.0-1.0/realT);
        double scale = 1.0/samplesChunk.cols();

        Eigen::SparseMatrix<double> tmp = a * a.transpose();

        for (int k=0; k<tmp.outerSize(); ++k)
            for (Eigen::SparseMatrix<double>::InnerIterator it(tmp,k); it; ++it) {
                double value = it.value();
                if((value!=value) || isinf(value)) {
                    std::cout << value << " " << it.row() << " "<< it.col() << std::endl;
                    continue;
                }
                (*A_)(it.row(),it.col()) = beta*(*A_)(it.row(),it.col()) + scale*value;
            }

        (*B_) = beta*(*B_) + scale*(samplesChunk*a.transpose());

        MatrixXd Dold = D.getData();

//        std::cout << "daft" << std::endl;
        VectorXd sum = VectorXd::Constant(a.cols(),0.0);
        for (int k=0; k<a.outerSize(); ++k) {
            for (Eigen::SparseMatrix<double>::InnerIterator it(a,k); it; ++it) {
                sum(it.col()) += 0.8*std::abs(it.value());
            }
        }

        static double r = 0.0;

        r += (0.5*((samplesChunk-D.getData()*a).array().square().matrix().colwise().sum())+sum.transpose()).sum();
//        r += 0.5*(D.getData().transpose()*D.getData()*(*A_)).trace()+(D.getData().transpose()*(*B_)).trace();

        update((*A_), (*B_), D);

        std::cout<< D.meta_->samples_ << " : " << r/D.meta_->samples_ << " : " << mse(Dold,D.getData()) << "   " << a.nonZeros()/a.outerSize() << std::endl;
        std::ostringstream o;
        o << "../../output/tmp/dict_tmp" << t << ".png";

        D.debugSaveImage( o.str().c_str() );


    }

}
