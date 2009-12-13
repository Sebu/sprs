#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include "patch.h"

class Transform
{
public:
    int _x, _y;
    int _seedX, _seedY;

    Patch* _seed;

    Transform(int x, int  y, int  seedX, int  seedY, Patch* seed);
};

#endif // TRANSFORMMAP_H
