#ifndef ORIENTATIONHISTOGRAM_H
#define ORIENTATIONHISTOGRAM_H

#include <opencv/cv.h>
#include <iostream>
#include <limits.h>

class OrientHist
{
public:
    int* _bins;
    int _numBins;
    OrientHist(IplImage* image, int numBins);

    int minDiff(OrientHist* other);
    int diff(OrientHist* other, int offset=0);
    float peak();



};

#endif // ORIENTATIONHISTOGRAM_H
