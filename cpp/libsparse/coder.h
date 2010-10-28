#ifndef CODER_H
#define CODER_H

//#include <vigra/matrix.hxx>

#include <eigen2/Eigen/Core>
#include <eigen2/Eigen/Sparse>
USING_PART_OF_NAMESPACE_EIGEN

class Dictionary;

class Coder
{
private:

public:
    Coder();
    virtual Eigen::SparseMatrix<float> encode(MatrixXf&, Dictionary&) = 0;
//    virtual vigra::Matrix<double> decode(vigra::Matrix<double>&, Dictionary&) = 0;
};

#endif // CODER_H
