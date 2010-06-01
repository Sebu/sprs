#ifndef EPIIMAGE_H
#define EPIIMAGE_H


#include <list>

#include "match.h"

class EpiImage
{
public:
    std::vector<Transform*> transforms_;

    int blocksx_, blocksy_, s_;

    cv::Mat texture_;
    std::list<Chart*> charts_;

    EpiImage();

    cv::Mat Texture();
    void genTexture();

    // load/save/reconstruct
    void reconstruct(cv::Mat&);
    void saveTexture(std::string);

    void load(std::string fileName);
    void save(std::string);

};

#endif // EPIIMAGE_H
