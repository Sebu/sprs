#include <fstream>

#include "match.h"
#include "cv_ext.h"
#include "matrix.h"


bool matchSorter(Match* i, Match* j) { return (i->error_ < j->error_ ); }

Match::Match(Patch* seed)
    : error_(0.0f), transformed_(0)
{
    if (!seed) return;
    seed->transScaleFlipMat_.copyTo(t_.transformMat_);
//    sourceImage_ = seed->sourceColor_;

}


void Match::calcHull() {
    double points[4][2] = { {-1, -1},
                            {block_->s_, -1},
                            {block_->s_, block_->s_},
                            {-1, block_->s_}
    };

    cv::Mat selection(t_.transformMat_, cv::Rect(0,0,3,2));
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

void Transform::load(std::ifstream& ifs) {
    for (int i=0; i<2; i++)
        for(int j=0; j<3; j++)
            ifs >> transformMat_.at<double>(i,j);
    ifs >> colorScale_[0] >> colorScale_[1] >> colorScale_[2];
}

void Transform::save(std::ofstream& ofs) {
    for (int i=0; i<2; i++)
        for(int j=0; j<3; j++)
            ofs << transformMat_.at<double>(i,j) << " ";
    ofs << colorScale_[0] << " " << colorScale_[1] << " " << colorScale_[2] << " ";
}

void Match::save(std::ofstream& ofs) {
    t_.save(ofs);
}


void Match::serialize(std::ofstream& ofs) {

    t_.save(ofs);
    ofs << error_ << " ";
}

void Match::deserialize(std::ifstream& ifs) {

    t_.load(ifs);
    ifs >> error_;

    calcHull();
}


cv::Mat Transform::warp(cv::Mat &src, uint s) {

    cv::Mat warped;
    cv::Mat selection(transformMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(src, warped, selection, cv::Size(s, s));

    return warped;
}

cv::Mat Match::warp() {
    return t_.warp(block_->sourceColor_, block_->s_);
}


cv::Mat Transform::reconstruct(cv::Mat &src, uint s) {
    cv::Mat warped = warp(src, s);

    // color scale
    //*/
    std::vector<cv::Mat> planes;
    split(warped, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= colorScale_.val[i];
    }
    merge(planes, warped);
    //*/

    return warped;
}

cv::Mat Match::reconstruct() {
   return t_.reconstruct(block_->sourceColor_, block_->s_);
}


