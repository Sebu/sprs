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


    inline	 float minDiff(OrientHistFast* other) {
        float angle=0;

        float min = FLT_MAX;
        for(uint j=0; j < numBins_; j++) {
            float sum=0;
            for (int i=0; i < numBins_; i++){
                sum += pow(bins_[i]-other->bins_[ (i+j) % numBins_ ], 2);
//                if (sum>=min) break;
            }
            if (sum<min) { min=sum; angle=j*factor_;}
        }

        return angle;
    }

    inline float diff(OrientHistFast* other, int offset) {
        float sum=0;
        for (int i=0; i < numBins_; i++){
            sum += pow(bins_[i]-other->bins_[ (i+offset) % numBins_ ], 2);
        }
        return sum;
    }




};

#endif // ORIENTATIONHISTOGRAMFAST_H
