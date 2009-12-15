#ifndef CV_EXT_H
#define CV_EXT_H

#include <opencv/cv.h>

const double PI = 3.14159265359;

IplImage*   copyBlock(IplImage *src, IplImage* dest, CvRect roiSrc, CvRect roiDest=cvRect(0,0,0,0));
IplImage*   subImage(IplImage *image, CvRect roi);
float       histogramMean(IplImage* img);

#endif // CV_EXT_H
