#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include <vector>
#include "matrix.h"
#include "patch.h"

class Patch;
class Tile;


class Transform {
public:
    cv::Scalar colorScale_;
    cv::Mat transformMat_;

    Transform() {
        colorScale_ = cv::Scalar::all(1.0f);
        transformMat_ = cv::Mat::eye(3,3,CV_64FC1);
    }

    cv::Mat warp(cv::Mat&, uint);
    cv::Mat reconstruct(cv::Mat&, uint);

    void load(std::ifstream&);
    void save(std::ofstream&);

};

class Match
{

public:

    // things to save aka transform
    Transform t_;
    bool transformed_;

    float error_;

    Patch* block_;

    Polygon hull_;
    Polygon bbox_;
    cv::Mat sourceImage_;

    Match(Patch* seed=0);


    bool isPatch();

    cv::Mat warp();
    cv::Mat reconstruct();

    void calcHull();

    void save(std::ofstream&);
    // (de-)serialize
    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);
};


bool matchSorter(Match* i, Match* j);

#endif // TRANSFORMMAP_H
