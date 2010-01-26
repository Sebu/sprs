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

    cv::Scalar colorScale;
    cv::Scalar error;

    int seedX, seedY;
    int seedW, seedH;
    float scale;

    std::vector<Patch*> overlapedPatches;


    Match(Patch* seed=0);

    void setSeed(Patch* seed);

    cv::Mat rotate();
    cv::Mat warp();
    cv::Mat reconstruct();

    Polygon getMatchbox();

    void deserialize(std::ifstream&);
    void serialize(std::ofstream&);
};

#endif // TRANSFORMMAP_H
