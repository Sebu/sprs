#ifndef TRANSFORMMAP_H
#define TRANSFORMMAP_H


#include <vector>
#include "matrix.h"
#include "patch.h"

class Patch;
class Square;

class Match
{

public:
    int seedX, seedY;
    int w_, h_;
    float scale_;
    cv::Mat transform_;
    cv::Scalar colorScale_;

    bool transformed_;
    float error_;

    Polygon hull_;

    Patch* block;
    cv::Mat sourceImage;


    cv::Mat warpMat;
    cv::Mat rotMat;

    // TODO: move to seed
    cv::Mat scaleMat;
    cv::Mat flipMat;
    cv::Mat translateMat;




    std::vector<Square*> coveredSquares_;



    Match(Patch* seed=0);


    bool isPatch();

    cv::Mat warp();
    cv::Mat reconstruct();

    void calcTransform();
    void calcHull();

    // (de-)serialize
    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);
};

#endif // TRANSFORMMAP_H
