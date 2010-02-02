#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include <vector>
#include "matrix.h"
#include "patch.h"

class Patch;

class Match
{
private:
    Patch* seed;

public:
    Patch* patch;
    cv::Mat sourceImage;

    cv::Mat warpMat;
    cv::Mat rotMat;
    cv::Mat scaleMat;
    cv::Mat flipMat;
    cv::Mat translateMat;

    cv::Scalar colorScale;
    float error;

    int seedX, seedY;
    int w_, h_;
    float scale;

    std::vector<Patch*> overlapedPatches;
//    Polygon hull;



    Match(Patch* seed=0);
    void setSeed(Patch* seed);


    bool isPatch();

    cv::Mat rotate();
    cv::Mat warp();
    cv::Mat reconstruct();

    Polygon getMatchbox();


    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);
};

#endif // TRANSFORMMAP_H
