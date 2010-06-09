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

    cv::Mat* baseImage_;

    std::vector<Patch*> satBlocks_;
    std::vector<Patch*> chartBlocks_;

    int benefit_;

    Chart(cv::Mat*);

    void caclBBox();

    cv::Mat getMap();


    //
    void save();
    //    void load();
};

#endif // EPITOME_H
