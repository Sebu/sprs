#ifndef CV_EXT_H
#define CV_EXT_H

#include <opencv/cv.h>


cv::Mat    copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest=cvRect(0,0,0,0));

#endif // CV_EXT_H
