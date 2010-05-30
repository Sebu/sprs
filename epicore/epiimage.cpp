#include "epiimage.h"

#include <fstream>
#include <opencv/cv.h>
#include <QtGlobal>

#include "cv_ext.h"

EpiImage::EpiImage()
{
}

void EpiImage::save(std::string fileName) {
    std::ofstream ofs( (fileName + ".map").c_str() );

    ofs << width_ << " " << height_ << " " << s_ << " ";

    foreach(Transform *transform, transforms_)
         transform->save(ofs);

    ofs.close();
}

void EpiImage::reconstruct(cv::Mat& img) {

    for(uint i=0; i<height_; i++) {
        for(uint j=0; j<width_; j++) {
            cv::Mat reconstruction(transforms_[i*width_+j]->reconstruct(texture_, s_));
            copyBlock(reconstruction, img, cv::Rect(0, 0, s_, s_), cv::Rect(i, j, s_, s_) );
        }
    }

}
