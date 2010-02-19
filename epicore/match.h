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
    int seedX_, seedY_;
    int s_;
    float scale_;
    cv::Scalar colorScale_;

    bool transformed_;
    float error_;

    Polygon hull_;

    Patch* block_;
    cv::Mat sourceImage_;


    cv::Mat transformMat_;
    cv::Mat warpMat_;
    cv::Mat rotMat_;

    // TODO: move to patch
    cv::Mat scaleMat_;
    cv::Mat flipMat_;
    cv::Mat translateMat_;




    std::vector<Tile*> coveredTiles_;



    Match(Patch* seed=0);


    bool isPatch();

    cv::Mat warp();
    cv::Mat reconstruct();

    void calcTransform();
    void calcHull();

    void save(std::ofstream&);
    // (de-)serialize
    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);
};


bool matchSorter(Match* i, Match* j);

#endif // TRANSFORMMAP_H
