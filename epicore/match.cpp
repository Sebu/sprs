#include <fstream>

#include "match.h"
#include "cv_ext.h"
#include "matrix.h"


bool matchSorter(Match* i, Match* j) { return (i->error_ < j->error_ ); }

Match::Match(Patch* seed)
    : colorScale_(cv::Scalar::all(1.0f)), error_(0.0f), s_(0), transformed_(0)
{

    rotMat_ = cv::Mat::eye(3,3,CV_64FC1);
    warpMat_ = cv::Mat::eye(3,3,CV_64FC1);
    transformMat_ = cv::Mat::eye(3,3,CV_64FC1);

    if (!seed) return;
//    seedX_ = seed->x_;
//    seedY_ = seed->y_;
//    scale_ = seed->scale_;
    s_ = seed->s_;

    transScaleFlipMat_ = seed->transScaleFlipMat_;

    sourceImage_ = seed->sourceColor_;

}


void Match::calcHull() {
    double points[4][2] = { {0 , 0},
                            {s_, 0},
                            {s_, s_},
                            {0, s_}
    };

    cv::Mat selection(transformMat_, cv::Rect(0,0,3,2));
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


void Match::save(std::ofstream& ofs) {
    for (int i=0; i<2; i++)
        for(int j=0; j<transformMat_.cols; j++)
            ofs << transformMat_.at<double>(i,j) << " ";
    ofs << colorScale_[0] << " " << colorScale_[1] << " " << colorScale_[2] << " ";
}


void Match::serialize(std::ofstream& ofs) {

    for (int i=0; i<2; i++)
        for(int j=0; j<transformMat_.cols; j++)
            ofs << transformMat_.at<double>(i,j) << " ";
    ofs << colorScale_[0] << " " << colorScale_[1] << " " << colorScale_[2] << " ";
    ofs << error_ << " ";
}

void Match::deserialize(std::ifstream& ifs) {

    for (int i=0; i<2; i++)
        for(int j=0; j<rotMat_.cols; j++)
            ifs >> transformMat_.at<double>(i,j);
    ifs >> colorScale_[0] >> colorScale_[1] >>  colorScale_[2];
    ifs >> error_;
    calcHull();
}



void Match::calcTransform() {
    transformMat_ =  warpMat_ * rotMat_ * transScaleFlipMat_;
}

cv::Mat Match::warp() {

    cv::Mat warped;
    cv::Mat selection(transformMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceImage_, warped, selection, cv::Size(s_, s_));

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


