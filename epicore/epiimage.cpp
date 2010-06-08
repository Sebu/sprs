#include "epiimage.h"

#include <fstream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QtGlobal>

#include "cv_ext.h"
#include "epitome.h"

EpiImage::EpiImage()
{
}

cv::Mat EpiImage::Texture() {
    if (texture_.empty()) genTexture();
    return texture_;
}

void EpiImage::saveTexture(std::string fileName) {

    // save texture
    cv::imwrite((fileName + ".epi.png").c_str(), Texture());

    // chart cache data
    std::ofstream ofs( (fileName + ".epi.txt").c_str() );
    foreach(Chart* c, charts_) {
        ofs << c->chartBlocks_.size() << " ";
        foreach(Patch* b,c->chartBlocks_)
            ofs << b->x_ << " " << b->y_ << " ";
    }
    ofs.close();
}

void EpiImage::save(std::string fileName) {
    std::ofstream ofs( (fileName + ".map").c_str() );

    ofs << blocksx_ << " " << blocksy_ << " " << s_ << " ";

    foreach(Transform *transform, transforms_)
         transform->save(ofs);

    ofs.close();
}


void EpiImage::load(std::string fileName) {
    std::ifstream ifs( (fileName + ".map").c_str() );

    ifs >> blocksx_ >> blocksy_ >> s_;

    for(int i=0; i< blocksx_*blocksy_; i++) {
        Transform *transform = new Transform();
        transform->load(ifs);
        transforms_.push_back(transform);
    }
    ifs.close();
}

void EpiImage::reconstruct(cv::Mat& img) {

    for(uint i=0; i<blocksy_; i++) {
        for(uint j=0; j<blocksx_; j++) {
            cv::Mat reconstruction(transforms_[i*blocksx_+j]->reconstruct(texture_, s_));
            copyBlock(reconstruction, img, cv::Rect(0, 0, s_, s_), cv::Rect(j*s_, i*s_, s_, s_) );
        }
    }

}


void EpiImage::genTexture() {
    texture_ = cv::Mat::ones(cv::Size(blocksx_*s_,blocksy_*s_), CV_8UC3);


    float color = 0;
    float step = 255.0f/charts_.size();
    foreach(Chart* epi, charts_) {
        foreach(Patch* block, epi->chartBlocks_) {
//                cv::rectangle(texture_, block->hull_.verts[0], block->hull_.verts[2],cv::Scalar((128-(int)color) % 255,(255-(int)color) % 255,(int)color,255),-1);
            cv::line(texture_, block->hull_.verts[0], block->hull_.verts[2], cv::Scalar((128-(int)color) % 255,(255-(int)color) % 255,(int)color,255));

            if(block->inChart_)
                    copyBlock(block->patchColor_, texture_, cv::Rect(0, 0, s_, s_), cv::Rect(block->x_, block->y_, s_, s_) );
        }
        color += step;
    }

}
