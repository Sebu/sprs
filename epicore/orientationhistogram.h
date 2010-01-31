#ifndef ORIENTATIONHISTOGRAM_H
#define ORIENTATIONHISTOGRAM_H

#include <opencv/cv.h>
#include <iostream>
#include <limits.h>

class Patch;

class OrientHist
{
public:
    float* _bins;
    int _numBins;
    OrientHist(cv::Mat& image, int numBins);

    void genOrientHists(Patch& patch);
    void genSingle(cv::Mat& image, int offset);

    float minDiff(OrientHist* other);
    float diff(OrientHist* other, int offset=0);



};

#endif // ORIENTATIONHISTOGRAM_H
