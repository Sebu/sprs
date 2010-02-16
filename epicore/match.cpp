#include <fstream>

#include "match.h"
#include "cv_ext.h"
#include "matrix.h"

Match::Match(Patch* seed)
    : colorScale_(cv::Scalar::all(1.0f)), error_(0.0f), seedX(0), seedY(0), w_(0), h_(0), scale_(0.0), transformed_(0)
{

    rotMat = cv::Mat::eye(3,3,CV_64FC1);
    warpMat = cv::Mat::eye(3,3,CV_64FC1);
    scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    flipMat = cv::Mat::eye(3,3,CV_64FC1);
    translateMat = cv::Mat::eye(3,3,CV_64FC1);
    transform_ = cv::Mat::eye(3,3,CV_64FC1);

    if (!seed) return;
    seedX = seed->x_;
    seedY = seed->y_;
    translateMat.at<double>(0,2)=-seedX;
    translateMat.at<double>(1,2)=-seedY;
    w_ = seed->w_;
    h_ = seed->h_;
    scale_ = seed->scale_;
    scaleMat.at<double>(0,0)/=scale_;
    scaleMat.at<double>(1,1)/=scale_;

    flipMat = seed->flipMat;

    sourceImage = seed->sourceImage_;

}


void Match::calcHull() {
    double points[4][2] = { {0 , 0},
                            {w_, 0},
                            {w_, h_},
                            {0, h_}
    };

    cv::Mat selection(transform_, cv::Rect(0,0,3,2));
    cv::Mat inverted;
    invertAffineTransform(selection, inverted);

    for(int i=0; i<4; i++ ) {
        cv::Mat p = (cv::Mat_<double>(3,1) << points[i][0], points[i][1], 1);

        cv::Mat a =  inverted * p;
        Vector2f point;
        point.m_v[0] = a.at<double>(0,0);
        point.m_v[1] = a.at<double>(0,1);
        hull_.verts.push_back(point);
    }

    // flipped? correct orientation
    Vector2f v1 = hull_.verts[0];
    Vector2f v2 = hull_.verts[1];
    Vector2f v3 = hull_.verts[2];
    float cross = ( (v2.m_v[0] - v1.m_v[0]) * (v3.m_v[1]-v2.m_v[1])) - ( (v2.m_v[1] - v1.m_v[1]) * (v3.m_v[0]-v2.m_v[0]) );
    if (cross<0.0)
        std::reverse(hull_.verts.begin(), hull_.verts.end());



}

void Match::deserialize(std::ifstream& ifs) {

    for (int i=0; i<2; i++)
        for(int j=0; j<rotMat.cols; j++)
            ifs >> transform_.at<double>(i,j);
    ifs >> colorScale_[0] >> colorScale_[1] >>  colorScale_[2];
    ifs >> error_;
    calcHull();
}

void Match::serialize(std::ofstream& ofs) {

    for (int i=0; i<2; i++)
        for(int j=0; j<transform_.cols; j++)
            ofs << transform_.at<double>(i,j) << " ";
    ofs << colorScale_[0] << " " << colorScale_[1] << " " << colorScale_[2] << " ";
    ofs << error_ << " ";
}


void Match::calcTransform() {
    transform_ =  warpMat * rotMat * flipMat * translateMat *  scaleMat;
}

cv::Mat Match::warp() {

    cv::Mat warped;
    cv::Mat selection(transform_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceImage, warped, selection, cv::Size(w_, h_));

    return warped;

}


cv::Mat Match::reconstruct() {

    cv::Mat warped = warp();

    // color scale
    std::vector<cv::Mat> planes;
    split(warped, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= colorScale_.val[i];
    }
    merge(planes, warped);



    return warped;
}


