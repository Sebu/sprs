#include <fstream>

#include "transformmap.h"
#include "cv_ext.h"

Transform::Transform(Patch* seed)
    : seed(0), colorScale(cv::Scalar::all(1.0f)), seedX(0), seedY(0), seedW(0), seedH(0), scale(0.0)
{
    setSeed(seed);
    rotMat = cv::Mat(2,3,CV_64FC1);
    cv::setIdentity(rotMat);
    warpMat = cv::Mat(2,3,CV_64FC1);
    cv::setIdentity(warpMat);

}

void Transform::setSeed(Patch* seed) {
    if (!seed) return;
    seedX = seed->x_;
    seedY = seed->y_;
    seedW = seed->w_;
    seedH = seed->h_;
    scale = seed->scale;
    sourceImage = seed->sourceImage_;
}

std::vector<cv::Point> Transform::getMatchbox() {
    double points[4][2] = { {seedX      , seedY},
                            {seedX+seedW, seedY},
                            {seedX+seedW, seedY+ seedH},
                            {seedX      , seedY+ seedH}
    };
    cv::Mat warpInv = cv::Mat::eye(3,3,CV_64FC1);
    cv::Mat selection( warpInv, cv::Rect(0,0,3,2) );
    cv::Mat rotInv;

    std::vector<cv::Point> newPoints;

    invertAffineTransform(warpMat, selection);
    invertAffineTransform(rotMat, rotInv);

    for(int i=0; i<4; i++ ) {
        cv::Mat p = (cv::Mat_<double>(3,1) << points[i][0], points[i][1], 1);

        cv::Mat a =  rotInv * (warpInv * p);

        cv::Point point;
        point.x = a.at<double>(0,0);
        point.y = a.at<double>(0,1);
        newPoints.push_back(point);

    }
    return newPoints;
}

void Transform::deserialize(std::ifstream& ifs) {

    ifs >> seedX >> seedY >> seedW >> seedH >> scale;
    ifs.ignore(8192, '\n');
    // omg O_o two times the same code
    for (int i=0; i<rotMat.rows; i++)
        for(int j=0; j<rotMat.cols; j++)
            ifs >> rotMat.at<double>(i,j);
    ifs.ignore(8192, '\n');
    for (int i=0; i<warpMat.rows; i++)
        for(int j=0; j<warpMat.cols; j++)
            ifs >> warpMat.at<double>(i,j);
    ifs.ignore(8192, '\n');
    ifs >> colorScale[0] >> colorScale[1] >>  colorScale[2];
    ifs.ignore(8192, '\n');
}

void Transform::serialize(std::ofstream& ofs) {
    ofs << seedX << " " << seedY << " " << seedW << " " << seedH << " " << scale << std::endl;

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

    cv::Mat rotated = sourceImage.clone();
    cv::warpAffine(sourceImage, rotated, rotMat, sourceImage.size());

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
    cv::Mat result( seedW, seedH, sourceImage.type() );
    copyBlock(warped, result, cvRect(seedX, seedY, seedW, seedH), cvRect(0, 0, seedW, seedH));

    // color scale
    std::vector<cv::Mat> planes;
    split(result, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= colorScale.val[i];
    }
    merge(planes, result);



    return result;
}


