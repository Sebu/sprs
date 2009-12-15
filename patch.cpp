#include "patch.h"
#include "transformmap.h"
#include "cv_ext.h"


float Patch::reconError(Transform* t) {

    Patch* other = t->_seed;

    IplImage* diff = cvCreateImage( cvSize(this->_w, this->_h),
                                    this->_patchImage->depth, this->_patchImage->nChannels );

    IplImage* mul = cvCreateImage( cvSize(this->_w, this->_h),
                                   this->_patchImage->depth, this->_patchImage->nChannels );



    // sqaure distance
    IplImage* reconstruction = t->reconstruct();


    // sqaure distance
    cvAbsDiff(this->_patchImage, reconstruction, diff);


    cvPow(diff, mul, 2);
    float sum = (float)cvSum(mul).val[0];




    // variance
    IplImage* varianceMap = cvCreateImage( cvSize(this->_w, this->_h),
                                   this->_patchImage->depth, this->_patchImage->nChannels );

    IplImage* mulVariance = cvCreateImage( cvSize(this->_w, this->_h),
                                   this->_patchImage->depth, this->_patchImage->nChannels );



    cvAbsDiffS(this->_patchImage, varianceMap, cvScalarAll(this->histMean()));
    cvPow(varianceMap, mulVariance, 2);
    float variance = (float)cvSum(mulVariance).val[0];


    float alpha = 1.0f; // 0 <= alpha <= 2
    float beta  = .02f;


    cvReleaseImage(&reconstruction);
    cvReleaseImage(&varianceMap);
    cvReleaseImage(&mulVariance);
    cvReleaseImage(&diff);
    cvReleaseImage(&mul);

    // reconError =  ( cvSum(mul).val[0] ) / ( pow( variance , alpha) + beta );
    return sum / variance;
}

bool Patch::trackFeatures(IplImage* other) {

    int count = 10;
    CvPoint2D32f pointsSrc[count];

    int w = _w; // _sourceImage->width;
    int h = _h; // _sourceImage->height;

    CvScalar white = cvScalar(255,255,25,255);


    IplImage* eig_image = cvCreateImage( cvSize(w, h), IPL_DEPTH_32F, 1);
    IplImage* temp_image = cvCreateImage( cvSize(w, h), IPL_DEPTH_32F, 1);


//    IplImage* mask = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
//    cvZero(mask);
//    cvRectangle(mask, cvPoint(_x, _y), cvPoint(_x+_w, _y+_h),  white, CV_FILLED);

    cvGoodFeaturesToTrack(this->_patchImage, eig_image, temp_image, pointsSrc, &count, .01, .01);

    CvPoint2D32f pointsDest[count];
    char status[count];

    cvCalcOpticalFlowPyrLK( _patchImage, other, 0, 0,
                            pointsSrc, pointsDest, count,
                            cvSize(3,3), 0,
                            status, 0,
                            cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 40, .1 ), 0 );

    if(count<3) { std::cout << "bad patch" << std::endl; return true; }


//    cvReleaseImage(&mask);
    cvReleaseImage(&eig_image);
    cvReleaseImage(&temp_image);

    CvPoint2D32f srcTri[3], destTri[3];

    int index = 0;
    for (int i=0; i<count; i++) {
        if (index>2) break;
        if(status[i]) {
            srcTri[index].x = pointsSrc[i].x;
            srcTri[index].y = pointsSrc[i].y;
            destTri[index].x = pointsDest[i].x;
            destTri[index].y = pointsDest[i].y;
            index++;
        }
    }
    if (index<=2) return false;

    cvGetAffineTransform( destTri, srcTri, this->_warpMat );

    /*
    for( int i = 0; i < count; i++) {
        int radius = h/25;
        cvCircle(this->_sourceImage,
                cvPoint((int)(pointsDest[i].x + 0.5f + other._x ),(int)(pointsDest[i].y + 0.5f + other._y)),
                radius,
                white);
    }
*/
    // _debugAlbum->fromIpl(this->_sourceImage);

    return true;

}
Transform* Patch::match(Patch& other) {

    Transform* t = new Transform(_x, _y, other._x, other._y, &other);


    // 4.1 translation, color scale
    // atm actually only contrast scale
    // drop when over bright
    float colorScale = this->histMean() / other.histMean();
    if(colorScale>1.25f) return 0;
    t->_colorScale = colorScale;




    // 4.1 rotation, orientation/gradient histogram
    float orientation = this->_orientHist->minDiff(other._orientHist);




    // apply initial rotation
    CvPoint2D32f center = cvPoint2D32f( other._x+(_w/2), other._y+(_h/2) );
    cv2DRotationMatrix( center, orientation, 1.0f, t->_rotMat );
    IplImage* rotated = t->rotate();

    IplImage* result = cvCreateImage( cvSize(_w, _h), _patchImage->depth, _patchImage->nChannels );
    copyBlock(rotated, result, cvRect(other._x, other._y, other._w, other._h), cvRect(0, 0, _w, _h));

    // 4.1 KLT matching
    if (!trackFeatures(result)) {
//        std::cout << _x << " " << _y << "drop?" << std::endl;
        return 0;
    }
    t->_warpMat = this->_warpMat;

    cvReleaseImage(&result);
    cvReleaseImage(&rotated);

    // 4 reconstruction error
    float reconstuctionError =  reconError(t);

    if (reconstuctionError>0.8f) return 0;

    std::cout << _x << " " << _y << " " <<  "color: " << colorScale << "\t\t orient.: " << orientation << "\t\t error: " << reconstuctionError << std::endl;

    return t;
}

Patch::Patch(IplImage* sourceImage, int x, int  y, int w, int h):
        _histMean(0.0f), _x(x), _y(y), _w(w), _h(h), _patchImage(0), _sourceImage(sourceImage)
{
    _warpMat = cvCreateMat(2,3,CV_32FC1);
    cvSetIdentity(_warpMat);
    _patchImage = subImage( sourceImage, cvRect(_x,_y,_w,_h) );
    _orientHist = new OrientHist(_patchImage, 36);
    setHistMean( histogramMean(_patchImage) );
}
