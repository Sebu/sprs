#include "orientationhistogram.h"
#include <opencv/cv.h>
#include "stdio.h"
#include "cv_ext.h"

#include "patch.h"

void OrientHist::genOrientHists(Patch& patch) {

    for(uint i=0; i<_numBins; i++) {
//        genSingle(patch,i);

    }
}


void OrientHist::genSingle(cv::Mat& image, int offset) {


//    cv::cvtColor(result, gray, CV_BGR2GRAY);


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

    cv::Mat contrast(image.size(), CV_32F);
    cv::Mat direction(image.size(), CV_32F);



    float sumContrast = 0 ;
    long count = 0 ;

    for(int y=0; y<image.rows-1; y++) {
        for (int x=0; x<image.cols-1; x++) {
            float pixel = image.at<uchar>(y, x);// / 256.0f;
            float pixel_x = image.at<uchar>(y, x+1);// / 256.0f;
            float pixel_y = image.at<uchar>(y+1, x);//   / 256.0f;

            float dx = pixel - pixel_x;
            float dy = pixel - pixel_y;


            direction.at<float>(y,x) = cv::fastAtan2(dx,dy);

            float ctmp = sqrt(dx*dx + dy*dy);
            contrast.at<float>(y,x) = ctmp;
            sumContrast += ctmp;
            count++;

        }
    }

    float threshold = (sumContrast / count) * 2.0f;

    for(int y=0; y<image.rows-1; y++)
        for (int x=0; x<image.cols-1; x++)
            if(contrast.at<float>(y,x) > threshold )
                _bins[  ( (int)direction.at<float>(y,x) / (360/_numBins)  ) ]++;
}


float OrientHist::minDiff(OrientHist* other) {
    float angle=0;

    float min = this->diff(other, 0);
    for(int j=0; j < this->_numBins; j++) {
        float sum = this->diff(other, j);
        if (sum<=min) { min=sum; angle=j*(360/_numBins); }
    }

    return angle;
}

float OrientHist::diff(OrientHist* other, int offset) {
    float sum=0;
    for (int i=0; i < _numBins; i++){
        sum += pow(this->_bins[i]-other->_bins[ (i+offset) % _numBins ], 2);
    }
    return sum;
}

OrientHist::OrientHist(cv::Mat& image, int numBins) : _bins(0), _numBins(numBins)
{
    _bins = new float[_numBins*_numBins];

    // init with 0s
    for(int i=0; i<_numBins*_numBins; i++) _bins[i]=0.0f;

    genSingle(image,0);

}
