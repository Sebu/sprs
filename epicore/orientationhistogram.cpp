#include "orientationhistogram.h"
#include <opencv/cv.h>
#include "stdio.h"
#include "cv_ext.h"

float OrientHist::peak() {
    int max = 0;
    for(int i=0; i<_numBins; i++) {
        if (_bins[i] > _bins[max]) max = i;
    }
    return max/35.0f;
}

int OrientHist::minDiff(OrientHist* other) {
    int angle=0;

    int min = INT_MAX;
    for(int j=0; j < this->_numBins; j++) {
        int sum = this->diff(other, j);
        if (sum<min) { min=sum; angle=j*10; }
    }

    return angle;
}

int OrientHist::diff(OrientHist* other, int offset) {
    int sum=0;
    for (int i=0; i < _numBins; i++){
        sum += pow(this->_bins[i]-other->_bins[ (i+offset) % _numBins ], 2);
    }
    return sum;
}

OrientHist::OrientHist(cv::Mat& image, int numBins) : _bins(0), _numBins(numBins)
{
    // preuso code
    //    for each pixel (x,y) in an image I
    //      {
    //         find the gradient of the pixel
    //             dx = I(x,y) - I(x+1,y)
    //             dy = I(x,y) - I(x,y+1)
    //         find gradient direction or orientation = arctan(dx, dy)
    //         find gradient magnitute = sqrt( dx*dx + dy*dy)
    //         if ( gradient magnitute > threshold )     // threshold is average_gradient_magnitute*2.0
    //           {
    //               find the group of gradient direction and increment the frequency
    //            }
    //      }
    //
    //      blur the initial histogram by [1 4 6 4 1] averaging filter.

    _bins = new int[_numBins];

    // init with 0
    for(int i=0; i<numBins; i++) _bins[i]=0;

    for(int y=0; y<image.rows-1; y++) {
        for (int x=0; x<image.cols-1; x++) {
            uchar pixel = image.at<uchar>(y, x);
            uchar pixel_x = image.at<uchar>(y, x+1);
            uchar pixel_y = image.at<uchar>(y+1, x);

            float dx = pixel - pixel_x;
            float dy = pixel - pixel_y;

            int dir = cv::fastAtan2(dx,dy);

            float magnitude = sqrt(dx*dx + dy*dy);

            float threshold = 1.0f;

            if(magnitude > threshold ) {
                _bins[ dir / (360/numBins)  ]++;
            }

        }

    }

}
