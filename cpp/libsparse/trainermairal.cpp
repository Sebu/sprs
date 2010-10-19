
#include "trainermairal.h"
#include "dictionary.h"
#include "coderlasso.h"
#include "coderomp.h"
#include "samples.h"

#include <iostream>

#include <vigra/multi_array.hxx>
typedef vigra::MultiArray<2, double>::difference_type Shape;

TrainerMairal::TrainerMairal()
{
}

void TrainerMairal::update(vigra::Matrix<double>& A, vigra::Matrix<double>& B, Dictionary& D) {

    for(int i=0; i < 10; i++) {
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

void TrainerMairal::train(Samples& samples, Dictionary& D, int iterations) {

    vigra::Matrix<double> A(D.getElementCount(), D.getElementCount()), B(D.getSignalSize(), D.getElementCount());

    // init A,B with 0
    A.init(0.0); B.init(0.0);

    int batch=100;
    CoderOMP coder;

    for(int t=0; t<iterations; t++) {

        // draw sample from trainig set
        vigra::Matrix<double> sample = samples.getData().subarray(Shape(0,t*batch), Shape(D.getSignalSize(),t*batch+batch));//columnVector(t);

        // sparse code sample
        vigra::Matrix<double> a = coder.code(sample, D);

        std::cout << "Iteration: " <<  t << std::endl;

        A = A + mmul(a,a.transpose());
        B = B + mmul(sample,a.transpose());
        // update step (algo. 2)
        update(A, B, D);
    }

}
