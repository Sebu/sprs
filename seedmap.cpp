
#include <opencv/cv.h>
#include <stdio.h>

#include "seedmap.h"
#include "cv_ext.h"


SeedMap::SeedMap( IplImage* image, int xgrid, int ygrid )
    : _xgrid(xgrid), _ygrid(ygrid), _sourceImage(0), _epitome_ipl(0), _mean_ipl(0), _orient_ipl(0)
{
    setImage(image);
}


void SeedMap::match(Patch &patch) {
    foreach (Patch* seed, this->_seeds ) {
        if ( patch.match(*seed) ) {
            _matches.append(seed);
            break; // take first match
        }

    }

}


Patch* SeedMap::at(int x, int y) {
    return this->_seeds.at(y*_width + x);
}


IplImage* SeedMap::epitomeIpl() {
    if (!_epitome_ipl) _epitome_ipl = cvCreateImage( cvSize(_sourceImage->width, _sourceImage->height), IPL_DEPTH_8U, 1);

    foreach (Patch* match, _matches) {
        copy_block(_sourceImage, _epitome_ipl, cvRect(match->_x, match->_y, match->_w, match->_h ) );
//        int radius = _sourceImage->width/25.0f;
//        cvCircle(_sourceImage, cvPoint((int)(match->_x + 0.5f),(int)(match->_y  + 0.5f)), radius, cvScalar(255,0,0));
    }


    return _epitome_ipl;
}


IplImage* SeedMap::meanIpl() {
    if (!_mean_ipl) _mean_ipl = cvCreateImage( cvSize(_width, _height), IPL_DEPTH_32F, 1);

    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){
            float mean = this->at(x,y)->histMean();
            cvSet2D( _mean_ipl, y, x, cvScalarAll(mean) );
        }
    }
    return _mean_ipl;
}


IplImage* SeedMap::orientIpl(float delta) {
    if (!_orient_ipl) _orient_ipl = cvCreateImage( cvSize(_width, _height), IPL_DEPTH_32F, 1);

    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){
            float max_f = at(x,y)->_orientHist->peak();
            cvSet2D( _orient_ipl, y, x, cvScalarAll(delta-max_f) );
        }
    }
    return _orient_ipl;
}



void SeedMap::setImage(IplImage* image) {
    _sourceImage = image;

    int w = 16;
    int h = 16;

    _width = (image->width-w) / _xgrid;
    _height = (image->height-h) / _ygrid;

    this->_seeds.clear(); // remove all old patches

    if(!_epitome_ipl) cvReleaseImage(&_epitome_ipl);
    if(!_mean_ipl) cvReleaseImage(&_mean_ipl);
    if(!_orient_ipl) cvReleaseImage(&_orient_ipl);

    // generate new patches
    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){
            Patch* seed = new Patch( _sourceImage, x*_xgrid, y*_ygrid, w, h );
            this->_seeds.append(seed);
        }
    }
}
