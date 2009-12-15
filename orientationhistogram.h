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

    float peak();


    int minDiff(OrientHist* other) {
        int angle=0;

        int min = INT_MAX;
        for(int j=0; j < this->_numBins; j++) {
            int sum = this->diff(other, j);
//            std::cout << sum << std::endl;
            if (sum<min) { min=sum; angle=j*10; }
        }

        return angle;
    }

    int diff(OrientHist* other, int offset=0) {
        int sum=0;
        for (int i=0; i < _numBins; i++){
            sum += pow(this->_bins[i]-other->_bins[ (i+offset) % _numBins ], 2);

            // std::cout << this->_bins[i] << " " << sum << std::endl;
        }
        return sum;
    }

};

#endif // ORIENTATIONHISTOGRAM_H
