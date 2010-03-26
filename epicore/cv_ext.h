#ifndef CV_EXT_H
#define CV_EXT_H

#define _BACKUP _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL
#define _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL 1
#include <opencv/cv.h>

cv::Mat getTransform( const std::vector<cv::Point2f> src, const std::vector<cv::Point2f> dst );
cv::Mat copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest=cvRect(0,0,0,0));

#endif // CV_EXT_H
