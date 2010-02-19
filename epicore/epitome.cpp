#include <opencv/highgui.h>
#include <limits.h>
#include <fstream>
#include <sstream>
#include <QList>
#include "epitome.h"


int Chart::staticCounter_ = 0;

Chart::Chart(cv::Mat* image) : maxX_(0), minX_(INT_MAX), maxY_(0), minY_(INT_MAX), baseImage_(image)
{
    id_ = staticCounter_++;
}


void Chart::caclDimensions() {
    // align
    for(uint i=0; i< reconTiles_.size(); i++) {
        Tile *tile = reconTiles_[i];

        // find min x and y
        if(tile->x_ < minX_) minX_ = tile->x_;
        if(tile->y_ < minY_) minY_ = tile->y_;
        if(tile->x_+4 > maxX_) maxX_ = tile->x_+4;
        if(tile->y_+4 > maxY_) maxY_ = tile->y_+4;

    }
    std::cout << minX_ << " " << minY_ << std::endl;
}

cv::Mat Chart::getMap() {
    caclDimensions();
    int width = maxX_-minX_;
    int height = maxY_-minY_;
    std::cout << width << " " << height << " " << minX_ << " " << minY_ << " " << reconTiles_.size() << std::endl;

    cv::Mat map = cv::Mat::zeros(height, width, CV_8UC3);
    for(uint i=0; i< reconTiles_.size(); i++) {
        Tile *tile = reconTiles_[i];
        // save seeds
        std::cout << tile->x_ - minX_ << " " << tile->y_ - minY_ << std::endl;
        cv::Mat selection(map, cv::Rect( tile->x_ - minX_, tile->y_ - minY_, 4, 4));
        (*baseImage_)(cv::Rect(tile->x_,tile->y_,4,4)).copyTo(selection);
    }
   return map;
}

void Chart::save()
{
    if (reconTiles_.empty()) return;

    caclDimensions();

    std::stringstream fileName;
    fileName << "../epitomes/" << id_;

    std::cout << fileName << std::endl;
    //*
    cv::Mat tilesImage = cv::Mat::zeros(4, reconTiles_.size()*4, CV_8UC3);



    std::ofstream ofs( (fileName.str() + ".txt").c_str() );
    ofs << maxX_-minX_ << " " << maxY_-minY_ << " ";
    for(uint i=0; i< reconTiles_.size(); i++) {
        Tile *tile = reconTiles_[i];
        // save seeds
        cv::Mat selection(tilesImage, cv::Rect(i*4, 0, 4, 4));
        (*baseImage_)(cv::Rect(tile->x_,tile->y_,4,4)).copyTo(selection);

        // save seed positions
        ofs << tile->x_ - minX_ << " " << tile->y_ - minY_ << " ";
    }
    ofs.close();

    cv::imwrite((fileName.str() + ".png"), tilesImage);
    //*/

}
