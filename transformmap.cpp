
#include "transformmap.h"
#include "cv_ext.h"

Transform::Transform(int  x, int  y, int  seedX, int  seedY, Patch* seed)
    : _x(x), _y(y), _seedX(seedX), _seedY(seedY), _seed(seed)
{
    _rotMat = cvCreateMat(2,3,CV_32FC1);

}

IplImage* Transform::rotate() {
    IplImage* rotated = cvCloneImage(_seed->_sourceImage);
    cvWarpAffine(_seed->_sourceImage, rotated, _rotMat );


//    IplImage* result = cvCreateImage( cvSize(_seed->_w, _seed->_h), _seed->_patchImage->depth, _seed->_patchImage->nChannels );
//    copyBlock(rotated, result, cvRect(_seed->_x, _seed->_y, _seed->_w, _seed->_h), cvRect(0, 0, _seed->_w, _seed->_h));


//    cvReleaseImage(&rotated);

    return rotated;
}

IplImage* Transform::reconstruct() {


    IplImage* rotated = rotate();
    IplImage* warped = cvCloneImage(rotated);


    // warp image
    cvWarpAffine(rotated, warped, _warpMat );

    // extract patch
    IplImage* result = cvCreateImage( cvSize(_seed->_w, _seed->_h), _seed->_patchImage->depth, _seed->_patchImage->nChannels );
    copyBlock(warped, result, cvRect(_seed->_x, _seed->_y, _seed->_w, _seed->_h), cvRect(0, 0, _seed->_w, _seed->_h));

    // color scale
    cvScale(result, result, _colorScale);

    cvReleaseImage(&rotated);
    cvReleaseImage(&warped);

    return result;
}


