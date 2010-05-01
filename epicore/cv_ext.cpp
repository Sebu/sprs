#include "cv_ext.h"
#include <iostream>


cv::Mat getTransform( const std::vector<cv::Point2f> src, const std::vector<cv::Point2f> dst )
{
    int size = src.size();
    cv::Mat M(2, 3, CV_64F), X(6, 1, CV_64F, M.data);
    double a[size*2*6], b[size*2];
    cv::Mat A(size*2, 6, CV_64F, a), B(size*2, 1, CV_64F, b);

    for( int i = 0; i < size; i++ )
    {
        int j = i*12;
        int k = i*12+6;
        a[j] = a[k+3] = src[i].x;
        a[j+1] = a[k+4] = src[i].y;
        a[j+2] = a[k+5] = 1;
        a[j+3] = a[j+4] = a[j+5] = 0;
        a[k] = a[k+1] = a[k+2] = 0;
        b[i*2] = dst[i].x;
        b[i*2+1] = dst[i].y;
    }

//    if (size==3)
      solve( A, B, X); //, cv::DECOMP_SVD);
//    else
//      solve( A, B, X, cv::DECOMP_SVD);
    return M;
}

cv::Mat copyBlock(cv::Mat& src, cv::Mat& dest, cv::Rect roiSrc, cv::Rect roiDest)
{
    if (roiDest.height == 0) roiDest = roiSrc;
    // change ROI
    cv::Mat region(dest, roiDest);
    src(roiSrc).copyTo(region);
    return dest;
}
