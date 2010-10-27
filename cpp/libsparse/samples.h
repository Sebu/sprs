#ifndef SAMPLES_H
#define SAMPLES_H

//#include <vigra/matrix.hxx>

#include <eigen2/Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN

#include "libsparse_global.h"

class Dictionary;

class LIBSPARSESHARED_EXPORT Samples
{
private:
   MatrixXf* data_;
public:
//   vigra::Matrix<double>* scaling_;
   int imageRows_, imageCols_;
   int rows_, cols_;
   int winSize_, channels_;
   Samples();
   MatrixXf & getData();
   bool loadImage(std::string& fileName, int winSize, int channels, int step=1);
   void saveImage(std::string& fileName, Dictionary& dict);
};

#endif // SAMPLES_H
