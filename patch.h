#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>

#include "orientationhistogram.h"

class Patch
{
private:
    float _histMean;



public:
    int _x;
    int _y;
    IplImage* _image;
    OrientHist _orientHist;

    Patch(IplImage* image);

    float histMean() { return _histMean; }
    void setHistMean(float hist_mean) { this->_histMean = hist_mean; }

};

#endif // SEED_H
