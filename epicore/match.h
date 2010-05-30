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

    cv::Mat warp(cv::Mat&, uint);
    cv::Mat reconstruct(cv::Mat&, uint);


    void save(std::ofstream&);

};

class Match
{

public:

    // things to save aka transform
    Transform t_;

    float error_;

    int s_;
    Patch* block_;
    cv::Mat transScaleFlipMat_;

    bool transformed_;
    Polygon hull_;
    Polygon bbox_;
    cv::Mat sourceImage_;


    cv::Mat warpMat_;
    cv::Mat rotMat_;


    std::vector<Patch*> coveredBlocks_;

    Match(Patch* seed=0);


    bool isPatch();

    cv::Mat warp();
    cv::Mat reconstruct();

    void calcTransform();
    void calcHull();
    void calcPos();

    void save(std::ofstream&);
    // (de-)serialize
    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);
};


bool matchSorter(Match* i, Match* j);

#endif // TRANSFORMMAP_H
