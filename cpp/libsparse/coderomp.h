#ifndef CODEROMP_H
#define CODEROMP_H

#include "coder.h"

class CoderOMP : public Coder
{
public:
    CoderOMP();
    Eigen::SparseMatrix<double> encode(MatrixXd& X, Dictionary& D);

};

#endif // CODEROMP_H
