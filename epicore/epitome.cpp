#include <opencv/highgui.h>
#include <limits.h>
#include <fstream>
#include <sstream>
#include <QList>
#include "epitome.h"


int Chart::staticCounter_ = 0;

Polyomino::Polyomino(uint angle, uint flip, uint s, Chart* chart) {

    cv::Mat centerMat = cv::Mat::eye(3,3,CV_32FC1);
    float xoffset = chart->bbox_.min.m_v[0] + (chart->bbox_.width()/2.0f);
    float yoffset = chart->bbox_.min.m_v[1] + (chart->bbox_.height()/2.0f);
    centerMat.at<float>(0,2)=-xoffset;
    centerMat.at<float>(1,2)=-yoffset;

    // flip :)
    cv::Mat flipMat = cv::Mat::eye(3,3,CV_32FC1);
    switch(flip) {
    case 1:
        flipMat.at<float>(0,0)=-1.0f;
        break;
    case 2:
        flipMat.at<float>(1,1)=-1.0f;
        break;
    default:
        break;
    }

    // rotation
    cv::Point2f center(0, 0);
    cv::Mat rotMat = cv::Mat::eye(3,3,CV_32FC1);
    cv::Mat rMat = cv::getRotationMatrix2D(center, angle, 1.0f);
    cv::Mat selection( rotMat, cv::Rect(0,0,3,2) );
    rMat.copyTo(selection);


    cv::Mat transMat = cv::Mat::eye(3,3,CV_32FC1);
    float whalf=0, hhalf=0;
    // HACK: for convenience :)
    if(angle==0 || angle == 180) {
        whalf = chart->bbox_.width()/2.0f;
        hhalf = chart->bbox_.height()/2.0f;
        w_ = ceil(chart->bbox_.width()/s);
        h_ = ceil(chart->bbox_.height()/s);
    } else {
        whalf = chart->bbox_.height()/2.0f;
        hhalf = chart->bbox_.width()/2.0f;
        w_ = ceil(chart->bbox_.height()/s);
        h_ = ceil(chart->bbox_.width()/s);
    }
    pgrid_ = new bool[w_*h_];
    for(uint i=0; i<w_*h_; i++) pgrid_[i] = false;

    transMat.at<float>(0,2)=whalf;
    transMat.at<float>(1,2)=hhalf;

    transform_ = transMat * rotMat * flipMat * centerMat;

    foreach(Patch* p, chart->chartBlocks_) {
        cv::Mat point = (cv::Mat_<float>(3,1) << p->x_+s/2, p->y_+s/2, 1.0f);
        cv::Mat a =  transform_ * point;
        uint x = a.at<float>(0,0) / s;
        uint y = a.at<float>(0,1) / s;
        pgrid_[y*w_ + x] = true;
    }

}



Chart::Chart(cv::Mat image) : benefit_(0), baseImage_(image)
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

void Chart::save(std::string fileName)
{
    if (chartBlocks_.empty()) return;

    std::stringstream saveName;
    saveName << fileName << ".chart" << id_;

    std::cout << saveName << std::endl;
    //*
    cv::Mat tilesImage = cv::Mat::zeros(12, chartBlocks_.size()*12, CV_8UC3);



    std::ofstream ofs( (saveName.str() + ".txt").c_str() );
//    ofs << maxX_-minX_ << " " << maxY_-minY_ << " ";
    for(uint i=0; i< chartBlocks_.size(); i++) {
        Patch *block = chartBlocks_[i];
        // save seeds
        cv::Mat selection(tilesImage, cv::Rect(i*block->s_ , 0, block->s_ , block->s_ ));
        baseImage_(cv::Rect(block->x_,block->y_, block->s_ , block->s_ )).copyTo(selection);

        // save seed positions
//        ofs << block->x_ - minX_ << " " << block->y_ - minY_ << " ";
    }
    ofs.close();

    cv::imwrite((saveName.str() + ".png"), tilesImage);
    //*/

}
