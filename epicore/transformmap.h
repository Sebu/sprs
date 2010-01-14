#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include "patch.h"

class Patch;

class Transform
{
public:

    cv::Mat warpMat;
    cv::Mat rotMat;

    Patch* seed;

    cv::Scalar colorScale;

    Transform(Patch* seed);

    cv::Mat rotate();
    cv::Mat warp();
    cv::Mat reconstruct();

    void serialize(std::ofstream&);
};

#endif // TRANSFORMMAP_H
