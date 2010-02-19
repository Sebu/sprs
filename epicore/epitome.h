#ifndef EPITOME_H
#define EPITOME_H

#include <vector>
#include "patch.h"

class Chart
{
public:
    static int staticCounter_;
    int id_;


    int maxX_, minX_, maxY_, minY_;

    cv::Mat* baseImage_;

    std::vector<Tile*> reconTiles_;

    Chart(cv::Mat*);
    void caclDimensions();

    cv::Mat getMap();


    //
    void save();
    //    void load();
};

#endif // EPITOME_H
