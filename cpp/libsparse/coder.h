#ifndef CODER_H
#define CODER_H

#include <vigra/matrix.hxx>

class Dictionary;

class Coder
{
private:

public:
    Coder();
    virtual vigra::Matrix<double> code(vigra::Matrix<double>&, Dictionary&) = 0;
};

#endif // CODER_H
