#ifndef ORIENTATIONHISTOGRAM_H
#define ORIENTATIONHISTOGRAM_H

#include <opencv/cv.h>
#include <iostream>
#include <limits.h>

class Patch;

class OrientHist
{
public:
    float* bins_;
    uint numBins_;
    float factor_;

    Patch* patch_;

    OrientHist(Patch*, int);

    void genOrientHists();
    void genSingle(cv::Mat&, int);

    float minDiff(OrientHist* other);
    float diff(OrientHist* other, int offset=0);



};

#endif // ORIENTATIONHISTOGRAM_H
