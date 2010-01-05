#include "cv_ext.h"
#include <iostream>


cv::Mat copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest)
{
//    std::cout << src.cols << " " << dest.cols << std::endl;
    if (roiDest.height == 0) roiDest = roiSrc;
    // change ROI
    cv::Mat region(dest, roiDest);
    src(roiSrc).copyTo(region);
    return dest;
}


cv::Mat subImage(cv::Mat& image, cv::Rect roi)
{
    return image(roi).clone();
}

cv::Scalar histogramMean(cv::Mat& img) {
    return cv::mean(img);
}
