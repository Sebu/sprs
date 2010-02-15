#ifndef EPITOME_H
#define EPITOME_H

#include <vector>
#include "patch.h"

class Chart
{
public:
    static int staticCounter_;
    int id_;

    int maxX, minX, maxY, minY;

    std::vector<Square*> reconSquares_;

    Chart();
    void caclDimensions();


    int grow(Match*);


    cv::Mat getMap();


    //
    void save();
    //    void load();
};

#endif // EPITOME_H
