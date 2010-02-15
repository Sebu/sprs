#include <opencv/highgui.h>
#include <limits.h>
#include <fstream>
#include <sstream>
#include <QList>
#include "epitome.h"


int Chart::staticCounter_ = 0;

Chart::Chart() : maxX(0), minX(INT_MAX), maxY(0), minY(INT_MAX)
{
    id_ = staticCounter_++;
}


int Chart::grow(Match* match) {

}


void Chart::caclDimensions() {
    // align
    for(uint i=0; i< reconSquares_.size(); i++) {
        //Patch *p = reconSquares_[i];

        // find min x and y
        //if(p->x_ < minX) minX = p->x_;
        //if(p->y_ < minY) minY = p->y_;
        //if(p->x_+p->w_ > maxX) maxX = p->x_+p->w_;
        //if(p->y_+p->h_ > maxY) maxY = p->y_+p->h_;

    }
    std::cout << minX << " " << minY << std::endl;
}

cv::Mat Chart::getMap() {
    caclDimensions();
    int width = maxX-minX;
    int height = maxY-minY;
    std::cout << width << " " << height << " " << minX << " " << minY << " " << reconSquares_.size() << std::endl;

    cv::Mat map = cv::Mat::zeros(height, width, CV_8UC3);
    for(uint i=0; i< reconSquares_.size(); i++) {
        Square *s = reconSquares_[i];
        // save seeds
        //std::cout << p->x_ - minX << " " << p->y_ - minY << std::endl;
        //cv::Mat selection(map, cv::Rect( p->x_ - minX, p->y_ - minY, p->w_, p->h_));
        //p->patchImage.copyTo(selection);
    }
   return map;
}

void Chart::save()
{
    if (reconSquares_.empty()) return;

    caclDimensions();

    std::stringstream fileName;
    fileName << "../epitomes/" << id_;

    std::cout << fileName << std::endl;
    /*
    Patch* first = reconSquares_.front();
    cv::Mat seedImage = cv::Mat::zeros(first->h_, reconSquares_.size()*first->w_, CV_8UC3);



    std::ofstream ofs( (fileName.str() + ".txt").c_str() );
    ofs << maxX-minX << " " << maxY-minY << " ";
    for(uint i=0; i< reconSquares_.size(); i++) {
        Patch *p = reconSquares_[i];
        // save seeds
        cv::Mat selection(seedImage, cv::Rect(i*p->w_, 0, p->w_, p->h_));
        p->patchImage.copyTo(selection);

        // save seed positions
        ofs << p->x_ - minX << " " << p->y_ - minY << " ";
    }
    ofs.close();

    cv::imwrite((fileName.str() + ".png"), seedImage);
    //*/

}
