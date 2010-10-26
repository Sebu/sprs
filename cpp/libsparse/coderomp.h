#ifndef CODEROMP_H
#define CODEROMP_H

#include "coder.h"
#include <vigra/matrix.hxx>

class CoderOMP : public Coder
{
public:
    CoderOMP();
    vigra::Matrix<double> encode(vigra::Matrix<double>& X, Dictionary& D);

};

#endif // CODEROMP_H
