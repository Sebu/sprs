#ifndef EPITOME_H
#define EPITOME_H

#include <vector>
#include "patch.h"

class Epitome
{
public:
    static int staticCounter_;
    int id_;

    int maxX, minX, maxY, minY;
    std::vector<Patch*> reconPatches;

    Epitome();
    void caclDimensions();
    void grow();
    cv::Mat getMap();


    //
    void save();
//    void load();
};

#endif // EPITOME_H
