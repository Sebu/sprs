#ifndef CODER_H
#define CODER_H

#include <Eigen/Core>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

//USING_PART_OF_NAMESPACE_EIGEN
using namespace Eigen;

class Dictionary;

class Coder
{
private:

public:
    int coeffs;
    double eps;

    Coder();
    virtual Eigen::SparseMatrix<double> encode(MatrixXd&, Dictionary&) = 0;
//    virtual vigra::Matrix<double> decode(vigra::Matrix<double>&, Dictionary&) = 0;
};

#endif // CODER_H
