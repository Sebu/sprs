#include "cv_ext.h"
#include <iostream>


cv::Mat copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest)
{
    if (roiDest.height == 0) roiDest = roiSrc;
    // change ROI
    cv::Mat region(dest, roiDest);
    src(roiSrc).copyTo(region);
    return dest;
}
