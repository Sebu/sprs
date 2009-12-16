#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include "patch.h"

class Patch;

class Transform
{
public:
    int _x, _y;
    int _seedX, _seedY;

    CvMat* _warpMat;
    CvMat* _rotMat;

    float  _colorScale;
    Patch* _seed;

    Transform(int x, int  y, int  seedX, int  seedY, Patch* seed);

    IplImage* rotate();
    IplImage* warp();
    IplImage* reconstruct();
};

#endif // TRANSFORMMAP_H
