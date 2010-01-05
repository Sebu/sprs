#ifndef CV_EXT_H
#define CV_EXT_H

#include <opencv/cv.h>

const double PI = 3.14159265359;

cv::Mat    copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest=cvRect(0,0,0,0));
cv::Mat    subImage(cv::Mat& image, cv::Rect roi);
cv::Scalar histogramMean(cv::Mat& img);

#endif // CV_EXT_H
