#ifndef CV_EXT_H
#define CV_EXT_H

#include <opencv/cv.h>

const double PI = 3.14159265359;

IplImage*   copy_block(IplImage *src, IplImage* dest, CvRect roi);
IplImage*   sub_image(IplImage *image, CvRect roi);
float       histogram_mean(IplImage* img);

#endif // CV_EXT_H
