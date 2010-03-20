#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include <vector>
#include "matrix.h"
#include "patch.h"

class Patch;
class Tile;



class Match
{

public:
//    int seedX_, seedY_;
    //    float scale_;

    cv::Scalar colorScale_;
    cv::Mat transformMat_;
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


    std::vector<Tile*> coveredTiles_;

    Match(Patch* seed=0);


    bool isPatch();

    cv::Mat warp();
    cv::Mat warpFull();
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
