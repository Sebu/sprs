#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include "patch.h"

class Patch;

class Transform
{
public:
    int _x, _y;
    int _seedX, _seedY;

    cv::Mat warpMat;
    cv::Mat rotMat;

    cv::Scalar colorScale;
    Patch* seed;

    Transform(int x, int  y, int  seedX, int  seedY, Patch* seed);

    cv::Mat rotate();
    cv::Mat warp();
    cv::Mat reconstruct();
};

#endif // TRANSFORMMAP_H
