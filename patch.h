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
    cv::Scalar histMean;


public:
    int _x, _y, _w, _h;

    int count;
    std::vector<cv::Point2f> pointsSrc;

    float scale;

    QList<Transform*>* matches;

    cv::Mat patchImage;
    cv::Mat sourceImage;
    OrientHist* orientHist;

    Patch(cv::Mat& sourceImage, int x=0, int y=0, int w=16, int h=16);

    void findFeatures();
    float reconError(Transform*);
    bool trackFeatures(Transform* t);
    Transform* match(Patch&, float error);

    cv::Scalar getHistMean() { return histMean; }
    void setHistMean(cv::Scalar _hist_mean) { this->histMean = _hist_mean; }

};

#endif // SEED_H
