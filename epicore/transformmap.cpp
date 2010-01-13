
#include "transformmap.h"
#include "cv_ext.h"

Transform::Transform(Patch* seed)
    : seed(seed), colorScale(cv::Scalar::all(1.0f))
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
    cv::Mat result( seed->w_, seed->h_, seed->patchImage.type() );
    copyBlock(warped, result, cvRect(seed->x_, seed->y_, seed->w_, seed->h_), cvRect(0, 0, seed->w_, seed->h_));

    // color scale
    std::vector<cv::Mat> planes;
    split(result, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= colorScale.val[i];
    }
    merge(planes, result);



    return result;
}


