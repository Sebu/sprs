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
//            std::cout << sum << std::endl;
        if (sum<min) { min=sum; angle=j*10; }
    }

    return angle;
}

int OrientHist::diff(OrientHist* other, int offset) {
    int sum=0;
    for (int i=0; i < _numBins; i++){
        sum += pow(this->_bins[i]-other->_bins[ (i+offset) % _numBins ], 2);

        // std::cout << this->_bins[i] << " " << sum << std::endl;
    }
    return sum;
}

OrientHist::OrientHist(IplImage* image, int numBins) : _bins(0), _numBins(numBins)
{
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

    for(int y=0; y<image->height-1; y++) {
        for (int x=0; x<image->width-1; x++) {
            CvScalar pixel = cvGet2D(image, y, x);
            CvScalar pixel_x = cvGet2D(image, y, x+1);
            CvScalar pixel_y = cvGet2D(image, y+1, x);

            float dx = pixel.val[0] - pixel_x.val[0];
            float dy = pixel.val[0] - pixel_y.val[0];

            int dir = (cvFastArctan(dx,dy)); // +PI) * 180.0f/PI;

            float magnitude = sqrt(dx*dx + dy*dy);

            float threshold = 2.0f;

            if(magnitude > threshold ) {
                _bins[ dir / (360/numBins) ]++;
            }

        }

    }

}
