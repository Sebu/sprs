#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>

#include "orientationhistogram.h"
#include "glwidget.h"
#include "transformmap.h"

class Transform;

class Patch
{
private:
    float _histMean;


public:
    int _x, _y, _w, _h;

    int count;
    CvPoint2D32f* _pointsSrc;

    float _scale;


    IplImage* _patchImage;
    IplImage* _sourceImage;
    OrientHist* _orientHist;

    Patch(IplImage* sourceImage, int x=0, int y=0, int w=16, int h=16);

    void findFeatures();
    float reconError(Transform*);
    bool trackFeatures(Transform* t);
    Transform* match(Patch&, float error);

    float histMean() { return _histMean; }
    void setHistMean(float hist_mean) { this->_histMean = hist_mean; }

};

#endif // SEED_H
