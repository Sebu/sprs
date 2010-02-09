#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <float.h>

#include "stdio.h"
#include "cv_ext.h"

#include "orientationhistogramfast.h"
#include "patch.h"



void OrientHistFast::genSingle(cv::Mat& image) {


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

    cv::Mat contrast = cv::Mat::zeros(image.size(), CV_32F);
    cv::Mat direction = cv::Mat::zeros(image.size(), CV_32F);



    float sumContrast = 0 ;
    long count = 0 ;

    for(int y=0; y<image.rows-1; y++) {
        for (int x=0; x<image.cols-1; x++) {
            float pixel = image.at<uchar>(y, x) / 255.0f;
            float pixel_x = image.at<uchar>(y, x+1) / 255.0f;
            float pixel_y = image.at<uchar>(y+1, x) / 255.0f;

            float dx = pixel - pixel_x;
            float dy = pixel - pixel_y;


            direction.at<float>(y,x) = cv::fastAtan2(dx ,dy);

            float ctmp = sqrt(dx*dx + dy*dy);
            contrast.at<float>(y,x) = ctmp;
            sumContrast += ctmp;
            count++;

        }
    }

    float threshold = (sumContrast / count) * 2.0f;

    for(int y=0; y<image.rows-1; y++) {
        for (int x=0; x<image.cols-1; x++) {
            if(contrast.at<float>(y,x) > threshold ) {
                int dir = (int) (direction.at<float>(y,x) / factor_);
                bins_[  dir  ]++;
            }
        }
    }
}


float OrientHistFast::minDiff(OrientHistFast* other) {
    float angle=0;

    float min = FLT_MAX;
    for(int j=0; j < this->numBins_; j++) {
        float sum = this->diff(other, j);
        if (sum<min) { min=sum; angle=j*factor_;}
    }

    return angle;
}

float OrientHistFast::diff(OrientHistFast* other, int offset) {
    float sum=0;
    for (int i=0; i < numBins_; i++){
        sum += pow(this->bins_[i]-other->bins_[ (i+offset) % numBins_ ], 2);
    }
    return sum;
}

OrientHistFast::OrientHistFast(Patch* patch, int numBins) : bins_(0), numBins_(numBins)
{
    patch_ = patch;
    bins_ = new float[numBins_];

    factor_ = 360/numBins_;
    // init with 0s
    for(int i=0; i<numBins_; i++) bins_[i]=0.0f;

    genSingle(patch->grayPatch);

}
