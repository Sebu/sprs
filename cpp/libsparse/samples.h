#ifndef SAMPLES_H
#define SAMPLES_H

#include <vigra/matrix.hxx>

#include "libsparse_global.h"

class LIBSPARSESHARED_EXPORT Samples
{
private:
   vigra::Matrix<double>* data_;
public:
   vigra::Matrix<double>* scaling_;
   int rowMax, colMax;
   Samples();
   vigra::Matrix<double> & getData();
   void load(std::string& fileName, int winSize, int channels);
};

#endif // SAMPLES_H
