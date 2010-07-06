#ifndef EPITOME_H
#define EPITOME_H

#include <vector>
#include "patch.h"


class Chart
{
public:
    static int staticCounter_;
    int id_;


    AABB bbox_;
    cv::Mat transform_;

    cv::Mat baseImage_;

    std::vector<Patch*> satBlocks_;
    std::vector<Patch*> chartBlocks_;

    int benefit_;

    Chart(cv::Mat);


    void caclBBox();

    cv::Mat getMap();

    void save(std::string fileName);


};


class Polyomino
{
public:
    cv::Mat transform_;
    uint w_, h_;
    bool* pgrid_;

    Polyomino(uint, uint, uint, Chart*);

    inline bool intersect(cv::SparseMat grid, int x, int y) {
        for(int yi=0; yi<h_; yi++)
            for(int xi=0; xi<w_; xi++)
                if(grid.value<uchar>((xi+x), (yi+y)) && pgrid_[yi*w_ + xi]) return true;

        return false;
    }


};

class PCost
{
public:
    uint cost_;
    uint x_, y_;
    Polyomino* polyo_;


};

#endif // EPITOME_H
