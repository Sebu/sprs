#ifndef SAMPLES_H
#define SAMPLES_H

//#include <vigra/matrix.hxx>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <Eigen/Core>

//USING_PART_OF_NAMESPACE_EIGEN
using namespace Eigen;

#include "sprscode_global.h"

class Dictionary;
class Coder;

class LIBSPARSESHARED_EXPORT Samples
{
private:
   MatrixXd* data_;
public:
   int imageRows_, imageCols_;
   int rows_, cols_, channels_;

   // compression settings
   int coeffs_;
   int quant_;
   int blockSize_;

   Samples();
   MatrixXd & getData();

   void normalize();

   void saveReconstruction(std::string& fileName, Dictionary& dict, Coder& coder, int step);
   void compress(std::string& fileName, Dictionary& dict, Coder& coder);
   void uncompress(std::string& fileName, Dictionary& dict);

   bool loadImage(std::string& fileName, int winSize, int channels, int step=1);
   bool sampleImage(cv::Mat inputImage, int step);

};

#endif // SAMPLES_H
