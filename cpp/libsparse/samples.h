#ifndef SAMPLES_H
#define SAMPLES_H

#include <vigra/matrix.hxx>

#include "libsparse_global.h"

class Dictionary;

class LIBSPARSESHARED_EXPORT Samples
{
private:
   vigra::Matrix<double>* data_;
public:
   vigra::Matrix<double>* scaling_;
   int imageRows_, imageCols_;
   int rows_, cols_;
   int winSize_, channels_;
   Samples();
   vigra::Matrix<double> & getData();
   void load(std::string& fileName, int winSize, int channels);
   void save(std::string& fileName, Dictionary& dict);
};

#endif // SAMPLES_H
