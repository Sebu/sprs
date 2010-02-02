#ifndef ORIENTATIONHISTOGRAMFAST_H
#define ORIENTATIONHISTOGRAMFAST_H

#include <opencv/cv.h>
#include <iostream>
#include <limits.h>

class Patch;

class OrientHistFast
{
public:
    float* bins_;
    int numBins_;
    float factor_;

    Patch* patch_;

    OrientHistFast(Patch*, int);

    void genOrientHists();
    void genSingle(cv::Mat&);

    float minDiff(OrientHistFast* other);
    float diff(OrientHistFast* other, int offset);



};

#endif // ORIENTATIONHISTOGRAMFAST_H
