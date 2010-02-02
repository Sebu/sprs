#include <fstream>

#include "match.h"
#include "cv_ext.h"
#include "matrix.h"

Match::Match(Patch* seed)
    : seed(0), colorScale(cv::Scalar::all(1.0f)), error(0.0f), seedX(0), seedY(0), w_(0), h_(0), scale(0.0)
{
    rotMat = cv::Mat::eye(3,3,CV_64FC1);
    warpMat = cv::Mat::eye(3,3,CV_64FC1);
    scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    flipMat = cv::Mat::eye(3,3,CV_64FC1);
    translateMat = cv::Mat::eye(3,3,CV_64FC1);

    setSeed(seed);

}

void Match::setSeed(Patch* seed) {
    if (!seed) return;
    seedX = seed->x_;
    seedY = seed->y_;
    translateMat.at<double>(0,2)=-seedX;
    translateMat.at<double>(1,2)=-seedY;
    w_ = seed->w_;
    h_ = seed->h_;
    scale = seed->scale;
    scaleMat.at<double>(0,0)/=scale;
    scaleMat.at<double>(1,1)/=scale;

    flipMat = seed->flipMat;

    sourceImage = seed->sourceImage_;
}

Polygon Match::getMatchbox() {
    double points[4][2] = { {0 , 0},
                            {w_, 0},
                            {w_, h_},
                            {0, h_}
    };
    Polygon box;

    cv::Mat transform = warpMat * rotMat * flipMat * translateMat * scaleMat;
    cv::Mat selection(transform, cv::Rect(0,0,3,2));
    cv::Mat inverted;
    invertAffineTransform(selection, inverted);

    for(int i=0; i<4; i++ ) {
        cv::Mat p = (cv::Mat_<double>(3,1) << points[i][0], points[i][1], 1);

       cv::Mat a =  inverted * p;

        Vector2f point;
        point.m_v[0] = a.at<double>(0,0);
        point.m_v[1] = a.at<double>(0,1);
        box.verts.push_back(point);

    }
    return box;
}

void Match::deserialize(std::ifstream& ifs) {

    ifs >> seedX >> seedY >> w_ >> h_ >> scale;
    translateMat.at<double>(0,2)=-seedX;
    translateMat.at<double>(1,2)=-seedY;
    scaleMat.at<double>(0,0)/=scale;
    scaleMat.at<double>(1,1)/=scale;

    ifs.ignore(8192, '\n');
    // omg O_o two times the same code
    for (int i=0; i<2; i++)
        for(int j=0; j<rotMat.cols; j++)
            ifs >> rotMat.at<double>(i,j);
    ifs.ignore(8192, '\n');
    for (int i=0; i<2; i++)
        for(int j=0; j<warpMat.cols; j++)
            ifs >> warpMat.at<double>(i,j);
    ifs.ignore(8192, '\n');
    ifs >> colorScale[0] >> colorScale[1] >>  colorScale[2];
    ifs.ignore(8192, '\n');
    ifs >> error;
    ifs.ignore(8192, '\n');
}

void Match::serialize(std::ofstream& ofs) {

    cv::Mat transform =  warpMat * rotMat * flipMat * translateMat *  scaleMat;
    for (int i=0; i<2; i++)
        for(int j=0; j<transform.cols; j++)
            ofs << transform.at<double>(i,j) << " ";
    ofs << "transform" << std::endl;
    ofs << colorScale[0] << " " << colorScale[1] << " " << colorScale[2] << std::endl;
    ofs << error << std::endl;
}


cv::Mat Match::rotate() {

    cv::Mat transform = rotMat * flipMat * translateMat * scaleMat;
    cv::Mat rotated;
    cv::Mat selection(transform, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceImage, rotated, selection, cv::Size(w_, h_));

    return rotated;

}

cv::Mat Match::warp() {

    cv::Mat transform =  warpMat * rotMat * flipMat * translateMat *  scaleMat;
    cv::Mat warped;
    cv::Mat selection(transform, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceImage, warped, selection, cv::Size(w_, h_));

    return warped;

}


cv::Mat Match::reconstruct() {

    cv::Mat warped = warp();

    // color scale
    std::vector<cv::Mat> planes;
    split(warped, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= colorScale.val[i];
    }
    merge(planes, warped);



    return warped;
}


