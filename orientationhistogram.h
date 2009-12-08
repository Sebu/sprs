#ifndef ORIENTATIONHISTOGRAM_H
#define ORIENTATIONHISTOGRAM_H

#include <opencv/cv.h>
#include <iostream>

class OrientHist
{
public:
    int* _bins;
    int _numBins;
    OrientHist(IplImage* image, int numBins);

    float peak();

    bool compare(OrientHist* other) const {
        int sum=0;
        for (int i=0; i < _numBins; i++){
            sum += pow(this->_bins[i]-other->_bins[i],2);

//            std::cout << this->_bins[i] << " " << sum << std::endl;
        }
//        std::cout << sum << std::endl;

        return (sum<300);
    }

};

#endif // ORIENTATIONHISTOGRAM_H
