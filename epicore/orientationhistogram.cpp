#include "orientationhistogram.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "stdio.h"
#include "cv_ext.h"

#include "patch.h"

void OrientHist::genOrientHists() {

    cv::Mat scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    scaleMat.at<double>(0,0)/=patch_->scale_;
    scaleMat.at<double>(1,1)/=patch_->scale_;

    cv::Mat translateMat = cv::Mat::eye(3,3,CV_64FC1);
    translateMat.at<double>(0,2)=-patch_->x_;
    translateMat.at<double>(1,2)=-patch_->y_;


    cv::Mat rotMat = cv::Mat::eye(3,3,CV_64FC1);

    for(uint i=0; i<numBins_; i++) {
        cv::Point2f center( (patch_->w_/2), (patch_->h_/2) );

        float orientation = i*factor_;

        cv::Mat rot = cv::getRotationMatrix2D(center, -orientation, 1.0f);
        cv::Mat selection( rotMat, cv::Rect(0,0,3,2) );
        rot.copyTo(selection);

        cv::Mat transform = rotMat * patch_->flipMat * translateMat *  scaleMat;
        cv::Mat rotPatch;
        cv::Mat selectionT(transform, cv::Rect(0,0,3,2));
        cv::warpAffine(patch_->sourceGray_, rotPatch, selectionT, cv::Size(patch_->w_, patch_->h_));
//        cv::imshow("test", rotPatch);
        genSingle(rotPatch, i);

    }
}


void OrientHist::genSingle(cv::Mat& image, int offset) {


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
            float pixel = image.at<uchar>(y, x); // / 255.0f;
            float pixel_x = image.at<uchar>(y, x+1);// / 255.0f;
            float pixel_y = image.at<uchar>(y+1, x);// / 255.0f;

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
                bins_[  offset*numBins_ + dir  ]++;
            }
        }
    }
}


float OrientHist::minDiff(OrientHist* other) {
    float angle=0;

    float min = 100000000000.0f;
    for(uint j=0; j < this->numBins_; j++) {
        float sum = this->diff(other, j);
        if (sum<=min) { min=sum; angle=j*factor_;}
    }

    return angle;
}

float OrientHist::diff(OrientHist* other, int offset) {
    float sum=0;
    for (uint i=0; i < numBins_; i++){
        sum += pow(this->bins_[i]-other->bins_[ offset*numBins_ + i], 2);
    }
    return sum;
}

OrientHist::OrientHist(Patch* patch, int numBins) : bins_(0), numBins_(numBins)
{
    patch_ = patch;
    bins_ = new float[numBins_*numBins_];

    factor_ = 360/numBins_;
    // init with 0s
    for(uint i=0; i<numBins_*numBins_; i++) bins_[i]=0.0f;

//    genSingle(image,0);
    genOrientHists();

}
