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

    for(int i=0; i<numBins; i++) _bins[i]=0;

    for(int y=0; y<image->height-1; y++) {
        for (int x=0; x<image->width-1; x++) {
            CvScalar pixel = cvGet2D(image, y, x);
            CvScalar pixel_x = cvGet2D(image, y, x+1);
            CvScalar pixel_y = cvGet2D(image, y+1, x);

            float dx = pixel.val[0] - pixel_x.val[0];
            float dy = pixel.val[0] - pixel_y.val[0];

            int dir = (atan2(dx,dy)+PI) * 180.0f/PI;

            float magnitude = sqrt(dx*dx + dy*dy);

            float threshold = 0.0f;

            if(magnitude > threshold ) {
                _bins[ dir / (360/numBins) ]++;
            }

        }
    }

    //for(int i=0; i<numBins; i++) printf("%i ", _bins[i]);
    //printf("\n");
}
