#ifndef LIBSPARSE_H
#define LIBSPARSE_H

#include <string>
#include <eigen2/Eigen/Core>
#include <eigen2/Eigen/Sparse>
USING_PART_OF_NAMESPACE_EIGEN

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
