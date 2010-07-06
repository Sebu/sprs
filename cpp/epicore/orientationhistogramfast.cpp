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

    cv::Mat contrast = cv::Mat::zeros(image.size(), CV_32FC1);
    cv::Mat direction = cv::Mat::zeros(image.size(), CV_32FC1);



    float sumContrast = 0 ;
    long count = 0 ;

    for(int y=0; y<image.rows-1; y++) {
        for (int x=0; x<image.cols-1; x++) {
            float pixel = image.at<uchar>(y, x) / 255.0f;
            float pixel_x = image.at<uchar>(y, x+1) / 255.0f;
            float pixel_y = image.at<uchar>(y+1, x) / 255.0f;

            float dx = pixel - pixel_x;
            float dy = pixel - pixel_y;


            direction.at<float>(y,x) = cv::fastAtan2(dy ,dx);

            float ctmp = sqrt(dx*dx + dy*dy);
//            float ctmp = dx*dx + dy*dy;
            contrast.at<float>(y,x) = ctmp;
            sumContrast += ctmp;
            count++;

        }
    }

    float threshold = (sumContrast / count) * 2.0f;

    float bins[numBins_];
    for(int i=0; i<numBins_; i++) bins[i]=0.0f;

    for(int y=0; y<image.rows-1; y++) {
        for (int x=0; x<image.cols-1; x++) {
            //if(contrast.at<float>(y,x) > threshold ) {
                int dir = round(direction.at<float>(y,x) / factor_);
                bins[  dir % numBins_  ] += contrast.at<float>(y,x);
            //}
        }
    }


    for(int i=0; i<numBins_; i++) {
        float n2 = bins[ (i-2) % numBins_]*1.0;
        float n1 = bins[ (i-1) % numBins_]*4.0;
        float o = bins[ i ]*6.0;
        float p1 = bins[ (i+1) % numBins_]*4.0;
        float p2 = bins[ (i+2) % numBins_]*1.0;
        bins_[i] = (n2 + n1 + o + p1 + p2)/16.0;
    }

}




OrientHistFast::OrientHistFast(Patch* patch, int numBins) : bins_(0), numBins_(numBins)
{
    bins_ = new float[numBins_];

    factor_ = 360.0f/numBins_;
    // init with 0s
    for(int i=0; i<numBins_; i++) bins_[i]=0.0f;

    cv::Mat patchColor;
    cv::Mat patchGray;
    cv::Mat selection(patch->transScaleFlipMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(patch->sourceColor_, patchColor , selection, cv::Size(patch->s_, patch->s_));
    // cache gray patch version
    cv::cvtColor(patchColor, patchGray, CV_RGB2GRAY);

    genSingle(patchGray);

}
