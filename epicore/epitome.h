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
    std::vector<Patch*> reconSeeds_;
    std::vector<Polygon*> segments_;

    Epitome();
    void caclDimensions();


    bool intersects(Patch*);
    bool intersects(Match*);
    int grow(Match*);


    cv::Mat getMap();


    //
    void save();
    //    void load();
};

#endif // EPITOME_H
