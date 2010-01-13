#ifndef CV_EXT_H
#define CV_EXT_H

#include <opencv/cv.h>

const double PI = 3.14159265359;

cv::Mat    copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest=cvRect(0,0,0,0));

#endif // CV_EXT_H
