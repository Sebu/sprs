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
    cvReleaseImage(&reconstruction);

    cvPow(diff, mul, 2);
    float sum = (float)cvSum(mul).val[0];

    cvReleaseImage(&diff);
    cvReleaseImage(&mul);

    // variance
    IplImage* varianceMap = cvCreateImage( cvSize(this->_w, this->_h),
                                   this->_patchImage->depth, this->_patchImage->nChannels );

    IplImage* mulVariance = cvCreateImage( cvSize(this->_w, this->_h),
                                   this->_patchImage->depth, this->_patchImage->nChannels );



    cvAbsDiffS(this->_patchImage, varianceMap, cvScalarAll(this->histMean()));
    cvPow(varianceMap, mulVariance, 2);
    float variance = (float)cvSum(mulVariance).val[0];

    cvReleaseImage(&varianceMap);
    cvReleaseImage(&mulVariance);

    float alpha = 1.0f; // 0 <= alpha <= 2
    float beta  = .02f;


    // reconError =  ( cvSum(mul).val[0] ) / ( pow( variance , alpha) + beta );
    return sum / variance;
}

void Patch::findFeatures() {
    _pointsSrc = new CvPoint2D32f[count];

    IplImage* eig_image = cvCreateImage( cvSize(_w, _h), IPL_DEPTH_32F, 1);
    IplImage* temp_image = cvCreateImage( cvSize(_w, _h), IPL_DEPTH_32F, 1);

    cvGoodFeaturesToTrack(this->_patchImage, eig_image, temp_image, _pointsSrc, &count, .005, .01);

    cvReleaseImage(&eig_image);
    cvReleaseImage(&temp_image);

}
bool Patch::trackFeatures(Transform* t) {

    if(count<3) {
//        std::cout << "bad patch" << std::endl;
        return true;
    }

    CvPoint2D32f pointsDest[count];
    char status[count];

    IplImage* rotated = t->rotate();
    IplImage* result = cvCreateImage( cvSize(_w, _h), _patchImage->depth, _patchImage->nChannels );
    copyBlock(rotated, result, cvRect(t->_seedX, t->_seedY, _w, _h), cvRect(0, 0, _w, _h));

    cvCalcOpticalFlowPyrLK( _patchImage, result, 0, 0,
                            _pointsSrc, pointsDest, count,
                            cvSize(3,3), 1,
                            status, 0,
                            cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 40, .1 ), 0 );

    cvReleaseImage(&rotated);
    cvReleaseImage(&result);

    CvPoint2D32f srcTri[3], destTri[3];

    int index = 0;
    for (int i=0; i<count; i++) {
        if(status[i]) {
            srcTri[index].x = _pointsSrc[i].x + t->_seedX;
            srcTri[index].y = _pointsSrc[i].y + t->_seedY;
            destTri[index].x = pointsDest[i].x + t->_seedX;
            destTri[index].y = pointsDest[i].y + t->_seedY;
            index++;
        }
        if (index>2) break;
    }
    if (index<2) return true;

    cvGetAffineTransform( destTri, srcTri, t->_warpMat );

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
Transform* Patch::match(Patch& other, float error) {

    Transform* transform = new Transform(_x, _y, other._x, other._y, &other);


    // 4.1 translation, color scale
    // atm actually only contrast scale
    // drop when over bright
    float colorScale = this->histMean() / other.histMean();
    if(colorScale>1.25f) { free(transform); return 0; }
    transform->_colorScale = colorScale;


    // 4.1 rotation, orientation/gradient histogram
    float orientation = this->_orientHist->minDiff(other._orientHist);


    // apply initial rotation
    CvPoint2D32f center = cvPoint2D32f( other._x+(_w/2), other._y+(_h/2) );
    cv2DRotationMatrix( center, orientation, 1.0f, transform->_rotMat );

    // 4.1 KLT matching
    if (!trackFeatures(transform)) { free(transform); return 0; }


    // 4 reconstruction error
    float reconstuctionError =  reconError(transform);

    if (reconstuctionError > error) { free(transform); return 0; }

//    std::cout << _x << " " << _y << " " <<  "color: " << transform->_colorScale << "\t\t orient.: " << orientation << "\t\t error: " << reconstuctionError;
//    std::cout << " " << other._scale;
//    if(_x==other._x && _y==other._y) std::cout << "\tfound myself!";
//    std::cout << std::endl;

    return transform;
}

Patch::Patch(IplImage* sourceImage, int x, int  y, int w, int h):
        _histMean(0.0f), _x(x), _y(y), _w(w), _h(h), count(4), _patchImage(0), _sourceImage(sourceImage)
{
    _patchImage = subImage( sourceImage, cvRect(_x,_y,_w,_h) );
    _orientHist = new OrientHist(_patchImage, 36);
    setHistMean( histogramMean(_patchImage) );
}
