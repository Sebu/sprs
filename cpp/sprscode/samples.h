#ifndef SAMPLES_H
#define SAMPLES_H

//#include <vigra/matrix.hxx>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <eigen2/Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN

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

   bool loadImage(std::string& fileName, int winSize, int channels, int step=1);
   void saveImage(std::string& fileName, Dictionary& dict, Coder& coder);
   bool sampleImage(cv::Mat inputImage, int step);

};

#endif // SAMPLES_H
