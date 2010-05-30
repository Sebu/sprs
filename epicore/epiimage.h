#ifndef EPIIMAGE_H
#define EPIIMAGE_H


#include <list>

#include "match.h"

class EpiImage
{
public:
    std::vector<Transform*> transforms_;

    int width_, height_, s_;
    cv::Mat texture_;


    EpiImage();

    // load/save/reconstruct
    void save(std::string);
    void reconstruct(cv::Mat&);

};

#endif // EPIIMAGE_H
