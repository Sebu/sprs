#ifndef ORIENTATIONHISTOGRAM_H
#define ORIENTATIONHISTOGRAM_H

#include <opencv/cv.h>

class OrientHist
{
public:
    int* _bins;
    int _numBins;
    OrientHist(IplImage* image, int numBins);

    float peak();
};

#endif // ORIENTATIONHISTOGRAM_H
