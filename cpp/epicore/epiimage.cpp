#include "epiimage.h"

#include <fstream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QtGlobal>

#include "cv_ext.h"
#include "epitome.h"


bool chartSizeSorter (Chart* i, Chart* j) { return (i->bbox_.area() > j->bbox_.area()); }
bool positionCostSorter (PCost* i, PCost* j) { return (i->cost_ < j->cost_); }

std::vector<Polyomino*> EpiImage::genPolyominos(Chart* chart) {
    std::vector<Polyomino*> polyos;

    for(int flip=0; flip<3; flip++) {
        for(int angle=0; angle<360; angle+=90)
            polyos.push_back( new Polyomino(angle, flip, s_, chart) );
    }


    return polyos;
}


std::list<PCost*> EpiImage::genPCost(std::vector<Polyomino*> polyos, uint width, uint height) {
    std::list<PCost*> pCosts;

    // create
    foreach(Polyomino* polyo, polyos) {
        for(uint y=0; y<height+1; y++) {
            for(uint x=0; x<width+1; x++) {
                PCost* p = new PCost();
                p->x_ = x;
                p->y_ = y;
                p->polyo_ = polyo;

                uint newWidth = std::max(width, x+polyo->w_);
                uint newHeight = std::max(height, y+polyo->h_);
                p->cost_ = newWidth * newHeight;
                pCosts.push_back(p);
            }
        }

    }


    pCosts.sort(positionCostSorter);
    return pCosts;
}

void EpiImage::pack() {

    int size[] = {blocksx_*20, blocksy_*20};
    cv::SparseMat grid(2, size, CV_8U);

    charts_.sort(chartSizeSorter);
    Chart* first = charts_.front();
    width_ = ceil(first->bbox_.width()/s_);
    height_ = ceil(first->bbox_.height()/s_);

    foreach(Chart* bestChart, charts_) {

        std::vector<Polyomino*> polyos = genPolyominos(bestChart);

        std::list<PCost*> pCosts = genPCost(polyos, width_, height_);


        bestChart->transform_ = cv::Mat::eye(3,3,CV_32FC1);

        foreach(PCost* pCost, pCosts)
        {
            Polyomino* p = pCost->polyo_;
//            std::cout << "try pack position" << pCost->x_ << " " << pCost->y_ << std::endl;


            if(p->intersect(grid, pCost->x_, pCost->y_)) continue;

            // save transformation in chart
            cv::Mat transMat = cv::Mat::eye(3,3,CV_32FC1);
            transMat.at<float>(0,2)=pCost->x_*s_;
            transMat.at<float>(1,2)=pCost->y_*s_;


            bestChart->transform_= transMat * p->transform_;

            // mark grid cells covered
            for(uint yi=0; yi<p->h_; yi++)
                for(uint xi=0; xi<p->w_; xi++) {
                if(p->pgrid_[yi*p->w_+xi]) {
//                    std::cout << "in use" << xi+pCost->x_ << " " << yi+pCost->y_ << std::endl;
                    grid.ref<uchar>(xi+pCost->x_,yi+pCost->y_) = 1;
                }
            }

            width_ = std::max(width_, pCost->x_+p->w_);
            height_ = std::max(height_, pCost->y_+p->h_);

            break;
        }


        foreach(PCost* c, pCosts) { delete c; }
        pCosts.clear();

        foreach(Polyomino* p, polyos) { delete p; }
        polyos.clear();
    }

}

EpiImage::EpiImage()
{
}

void EpiImage::saveTexture(std::string fileName) {

    // save texture
    cv::imwrite((fileName + ".epi.png").c_str(), Texture());
    cv::imwrite((fileName + ".debugepi.png").c_str(), DebugTexture());

}

void EpiImage::save(std::string fileName) {
    std::ofstream ofs( (fileName + ".map").c_str() );

    ofs << blocksx_ << " " << blocksy_ << " " << s_ << " ";

    foreach(Transform *transform, transforms_)
        transform->save(ofs);

    ofs.close();
}


void EpiImage::load(std::string fileName) {
    std::cout << fileName << std::endl;
    std::ifstream ifs( (fileName + ".map").c_str() );
    texture_ = cv::imread( (fileName + ".epi.png").c_str());

    ifs >> blocksx_ >> blocksy_ >> s_;

    for(int i=0; i< blocksx_*blocksy_; i++) {
        Transform *transform = new Transform();
        transform->load(ifs);
        transforms_.push_back(transform);
    }
    ifs.close();
}

void EpiImage::saveRecontruction(std::string fileName) {

    cv::Mat img = cv::Mat::ones(cv::Size(blocksx_*s_, blocksy_*s_), CV_8UC3);
    reconstruct(img);
    cv::imwrite((fileName + ".recon.png").c_str(), img);
}

void EpiImage::reconstruct(cv::Mat& img) {

    for(uint i=0; i<blocksy_; i++) {
        for(uint j=0; j<blocksx_; j++) {
            cv::Mat reconstruction(transforms_[i*blocksx_+j]->reconstruct(texture_, s_));
            copyBlock(reconstruction, img, cv::Rect(0, 0, s_, s_), cv::Rect(j*s_, i*s_, s_, s_) );
        }
    }

}

cv::Mat EpiImage::DebugTexture() {
    if (debugTexture_.empty()) genDebugTexture();
    return debugTexture_;
}

void EpiImage::genDebugTexture() {
    debugTexture_ = cv::Mat::ones(cv::Size(blocksx_*s_, blocksy_*s_), CV_8UC3);

    cv::rectangle(debugTexture_, cv::Point(0,0), cv::Point(debugTexture_.cols, debugTexture_.rows),cv::Scalar(255,0,255),-1);

     foreach(Chart* epi, charts_) {
        foreach(Patch* block, epi->chartBlocks_) {
            if(!block->inChart_) continue;

            for(int srcY=block->y_; srcY<block->y_+s_; srcY++) {
                for(int srcX=block->x_; srcX<block->x_+s_; srcX++) {
                    cv::Mat p = (cv::Mat_<float>(3,1) << srcX, srcY, 1);
                    cv::Mat a =   p;
                    debugTexture_.at<cv::Vec3b>(a.at<float>(0,1), a.at<float>(0,0)) = block->sourceColor_.at<cv::Vec3b>(srcY, srcX);
                }

            }

        }
    }
}


cv::Mat EpiImage::Texture() {
    if (texture_.empty()) genTexture();
    return texture_;
}


void EpiImage::genTexture() {

    pack();

    texture_ = cv::Mat::ones(cv::Size(width_*s_, height_*s_), CV_8UC3);
    cv::rectangle(texture_, cv::Point(0,0), cv::Point(texture_.cols, texture_.rows),cv::Scalar(255,0,255),-1);

    foreach(Chart* epi, charts_) {
        foreach(Patch* block, epi->chartBlocks_) {
            if(!block->inChart_) continue;
            for(int srcY=block->y_; srcY<block->y_+s_; srcY++) {
                for(int srcX=block->x_; srcX<block->x_+s_; srcX++) {
                    cv::Mat p = (cv::Mat_<float>(3,1) << srcX, srcY, 1);
                    cv::Mat a =  epi->transform_ * p;
                    texture_.at<cv::Vec3b>(a.at<float>(0,1), a.at<float>(0,0)) = block->sourceColor_.at<cv::Vec3b>(srcY, srcX);
                }

            }

        }
    }

}
