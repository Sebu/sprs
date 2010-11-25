#ifndef CODERLASSO_H
#define CODERLASSO_H

#include "libsparse_global.h"
#include "coder.h"

class LIBSPARSESHARED_EXPORT CoderLasso : public Coder
{
public:
    CoderLasso();
    Eigen::SparseMatrix<double> encode(MatrixXd&, Dictionary&);
};

#endif // CODERLASSO_H
