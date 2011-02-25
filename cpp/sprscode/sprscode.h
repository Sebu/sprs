#ifndef LIBSPARSE_H
#define LIBSPARSE_H

#include <string>
#include <Eigen/Core>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>
//USING_PART_OF_NAMESPACE_EIGEN
using namespace Eigen;

#include "sprscode_global.h"

typedef struct {
    unsigned short width_;
    unsigned short height_;
    unsigned char depth_;
    unsigned char blockSize_;
    unsigned char quant_;
    unsigned char coeffs_;

} SprsHeader;

class LIBSPARSESHARED_EXPORT Sprscode {
public:
    SprsHeader header_;

    int shiftNum_;
    int indicesNum_;
    int coeffsNum_;

    unsigned char* shift_;
    unsigned short* indices_;
    char* coeffs_;


    Sprscode(int width, int height, int depth, int blockSize, int coeffsNum);
    void save(std::string& fileName);
    void load(std::string& fileName);
    void compress(VectorXd& shift, Eigen::SparseMatrix<double>& A);
    void uncompress(VectorXd& shift, Eigen::SparseMatrix<double>& A);
};

#endif // LIBSPARSE_H
