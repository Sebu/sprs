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

    inline float minDiff(OrientHist* other) {
        float angle=0;

        float min = FLT_MAX;
        for(uint j=0; j < this->numBins_; j++) {
            float sum = this->diff(other, j);
            if (sum<min) { min=sum; angle=j*factor_;}
        }

        return angle;
    }

    inline float diff(OrientHist* other, int offset) {
        float sum=0;
        for (uint i=0; i < numBins_; i++){
            sum += pow(this->bins_[i] - other->bins_[ offset*numBins_ + i], 2);
        }
        return sum;
    }

};

#endif // ORIENTATIONHISTOGRAM_H
