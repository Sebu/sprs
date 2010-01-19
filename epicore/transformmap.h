#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include "patch.h"

class Patch;

class Transform
{
private:
    Patch* seed;

public:
    cv::Mat sourceImage;

    cv::Mat warpMat;
    cv::Mat rotMat;
    cv::Scalar colorScale;
    float scale;
    int seedX, seedY;
    int seedW, seedH;


    Transform(Patch* seed=0);

    void setSeed(Patch* seed);

    cv::Mat rotate();
    cv::Mat warp();
    cv::Mat reconstruct();

    std::vector<cv::Point> getMatchbox();

    void deserialize(std::ifstream&);
    void serialize(std::ofstream&);
};

#endif // TRANSFORMMAP_H
