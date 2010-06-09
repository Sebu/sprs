#include <opencv/highgui.h>
#include <limits.h>
#include <fstream>
#include <sstream>
#include <QList>
#include "epitome.h"


int Chart::staticCounter_ = 0;

Chart::Chart(cv::Mat* image) : benefit_(0), baseImage_(image)
{
    id_ = staticCounter_++;
}


void Chart::caclBBox() {
    // align
    for(uint i=0; i< chartBlocks_.size(); i++) {
        Patch *block = chartBlocks_[i];

        // find min x and y
        if(block->x_ < bbox_.min.m_v[0]) bbox_.min.m_v[0] = block->x_;
        if(block->y_ < bbox_.min.m_v[1]) bbox_.min.m_v[1] = block->y_;
        if(block->x_+block->s_-1 > bbox_.max.m_v[0]) bbox_.max.m_v[0] = block->x_+block->s_-1;
        if(block->y_+block->s_-1 > bbox_.max.m_v[1]) bbox_.max.m_v[1] = block->y_+block->s_-1;

    }
}



cv::Mat Chart::getMap() {
    caclBBox();
    int width = bbox_.width();
    int height = bbox_.height();
    std::cout << width << " " << height << " " << chartBlocks_.size() << std::endl;

    cv::Mat map = cv::Mat::zeros(height, width, CV_8UC3);
    for(uint i=0; i< chartBlocks_.size(); i++) {
        Patch *block = chartBlocks_[i];
        // save seeds
//        std::cout << block->x_ - minX_ << " " << block->y_ - minY_ << std::endl;
//        cv::Mat selection(map, cv::Rect( block->x_ - minX_, block->y_ - minY_, block->s_ , block->s_ ));
//        (*baseImage_)(cv::Rect(block->x_,block->y_, block->s_ , block->s_ )).copyTo(selection);
    }
   return map;
}

void Chart::save()
{
    if (chartBlocks_.empty()) return;

    caclBBox();

    std::stringstream fileName;
    fileName << "../epitomes/" << id_;

    std::cout << fileName << std::endl;
    //*
    cv::Mat tilesImage = cv::Mat::zeros(12 , chartBlocks_.size()*12, CV_8UC3);



    std::ofstream ofs( (fileName.str() + ".txt").c_str() );
//    ofs << maxX_-minX_ << " " << maxY_-minY_ << " ";
    for(uint i=0; i< chartBlocks_.size(); i++) {
        Patch *block = chartBlocks_[i];
        // save seeds
        cv::Mat selection(tilesImage, cv::Rect(i*block->s_ , 0, block->s_ , block->s_ ));
        (*baseImage_)(cv::Rect(block->x_,block->y_, block->s_ , block->s_ )).copyTo(selection);

        // save seed positions
//        ofs << block->x_ - minX_ << " " << block->y_ - minY_ << " ";
    }
    ofs.close();

    cv::imwrite((fileName.str() + ".png"), tilesImage);
    //*/

}
