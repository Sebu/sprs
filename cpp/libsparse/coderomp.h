#ifndef CODEROMP_H
#define CODEROMP_H

#include "coder.h"
#include <vigra/matrix.hxx>

class CoderOMP : public Coder
{
public:
    CoderOMP();
    Eigen::SparseMatrix<float> encode(MatrixXf& X, Dictionary& D);

};

#endif // CODEROMP_H
