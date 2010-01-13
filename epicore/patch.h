#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>

#include "orientationhistogram.h"
#include "transformmap.h"

class Transform;

class Patch
{
private:
    cv::Scalar histMean;


public:
    int x_, y_, w_, h_;

    int count;
    std::vector<cv::Point2f> pointsSrc;

    float scale;

    std::vector<Transform*>* matches;

    cv::Mat patchImage;
    cv::Mat sourceImage_;
    OrientHist* orientHist;

    Patch(cv::Mat& sourceImage, int x=0, int y=0, int w=16, int h=16);

    bool isPatch();

    void findFeatures();
    float reconError(Transform*);
    bool trackFeatures(Transform* t);
    Transform* match(Patch&, float error);

    cv::Scalar getHistMean() { return histMean; }
    void setHistMean(cv::Scalar _hist_mean) { this->histMean = _hist_mean; }

    void serialize(std::ofstream&);

};

#endif // SEED_H
