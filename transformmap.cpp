
#include "transformmap.h"
#include "cv_ext.h"

Transform::Transform(int  x, int  y, int  seedX, int  seedY, Patch* seed)
    : _x(x), _y(y), seedX(seedX), seedY(seedY), seed(seed), colorScale(1.0f)
{
    rotMat = cv::Mat(2,3,CV_64FC1);
    cv::setIdentity(rotMat);
    warpMat = cv::Mat(2,3,CV_64FC1);
    cv::setIdentity(warpMat);

}

cv::Mat Transform::rotate() {

    cv::Mat rotated = seed->sourceImage.clone();
    cv::warpAffine(seed->sourceImage, rotated, rotMat, seed->sourceImage.size());

    return rotated;

}

cv::Mat Transform::warp() {

    cv::Mat rotated = rotate();
    cv::Mat warped = rotated.clone();
    // warp image
    cv::warpAffine(rotated, warped, warpMat, rotated.size());

    return warped;

}


cv::Mat Transform::reconstruct() {

    cv::Mat warped = warp();

    // extract patch
    cv::Mat result( seed->_w, seed->_h, seed->patchImage.type() );
    copyBlock(warped, result, cvRect(seed->_x, seed->_y, seed->_w, seed->_h), cvRect(0, 0, seed->_w, seed->_h));

    // color scale
    std::vector<cv::Mat> planes;
    split(result, planes);
    for(unsigned int i=0; i<planes.size(); i++) {
        planes[i] *= colorScale.val[i];
    }
    merge(planes, result);



    return result;
}


