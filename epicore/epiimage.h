#ifndef EPIIMAGE_H
#define EPIIMAGE_H


#include <list>

#include "match.h"


class Polyomino;
class PCost;

class EpiImage
{
public:
    std::vector<Transform*> transforms_;

    int blocksx_, blocksy_, s_;

    uint width_, height_;

    cv::Mat texture_;
    std::list<Chart*> charts_;

    EpiImage();

    cv::Mat Texture();
    void genTexture();


    // chart packing
    void pack();
    std::vector<Polyomino*> genPolyominos(Chart* chart);
    std::list<PCost*> genPCost(std::vector<Polyomino*>,uint, uint);
    bool intersect(Chart*);




    // load/save/reconstruct
    void reconstruct(cv::Mat&);
    void saveTexture(std::string);

    void load(std::string fileName);
    void save(std::string);
    void saveRecontruction(std::string);

};

#endif // EPIIMAGE_H
