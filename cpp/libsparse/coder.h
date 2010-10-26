#ifndef CODER_H
#define CODER_H

#include <vigra/matrix.hxx>

class Dictionary;

class Coder
{
private:

public:
    Coder();
    virtual vigra::Matrix<double> encode(vigra::Matrix<double>&, Dictionary&) = 0;
//    virtual vigra::Matrix<double> decode(vigra::Matrix<double>&, Dictionary&) = 0;
};

#endif // CODER_H
