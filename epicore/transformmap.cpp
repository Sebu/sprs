#include <fstream>

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


void Transform::serialize(std::ofstream& ofs) {
    ofs << seed->x_ << " " << seed->y_ << " " << seed->scale << std::endl;

    // omg O_o two times the same code
    for (int i=0; i<rotMat.rows; i++)
        for(int j=0; j<rotMat.cols; j++)
            ofs << rotMat.at<double>(i,j) << " ";
    ofs << "rotation" << std::endl;
    for (int i=0; i<warpMat.rows; i++)
        for(int j=0; j<warpMat.cols; j++)
            ofs << warpMat.at<double>(i,j) << " ";
    ofs << "warp" << std::endl;

    ofs << colorScale[0] << " " << colorScale[1] << " " << colorScale[2] << std::endl;
}

cv::Mat Transform::rotate() {

    cv::Mat rotated = seed->sourceImage_.clone();
    cv::warpAffine(seed->sourceImage_, rotated, rotMat, seed->sourceImage_.size());

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


