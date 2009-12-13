
#include <opencv/cv.h>
#include <stdio.h>

#include "seedmap.h"
#include "cv_ext.h"


SeedMap::SeedMap( IplImage* image, int xgrid, int ygrid )
    : _xgrid(xgrid), _ygrid(ygrid), _sourceImage(0)
{
    setImage(image);
}


void SeedMap::match(Patch &patch) {
    foreach (Patch* seed, this->_seeds ) {
        if ( patch.match(*seed) ) {
            _matches.append(seed);
            //            std::cout << patch._x << " " << patch._y << " " << seed->_x << " " << seed->_y << std::endl;
            Transform* t = new Transform(patch._x, patch._y, seed->_x, seed->_y,seed);
            _transforms.append(t);

            break; // take first match
        }

    }

}


Patch* SeedMap::at(int x, int y) {
    return this->_seeds.at(y*_width + x);
}


IplImage* SeedMap::reconstructIpl() {
    _debugImages["reconstuct"] = cvCreateImage( cvSize(_sourceImage->width, _sourceImage->height), IPL_DEPTH_8U, 1);

    foreach (Transform* t, _transforms) {
        //        std::cout << t->_seedX << " " << t->_seedY << " " << t->_x << " "  << t->_y << std::endl;
        int w = t->_seed->_w;
        int h = t->_seed->_h;

        copy_block(t->_seed->_sourceImage, _debugImages["reconstuct"] , cvRect(t->_seedX, t->_seedY, w, h), cvRect(t->_x, t->_y, w, h) );
    }

    return _debugImages["reconstuct"];
}


IplImage* SeedMap::epitomeIpl() {
    _debugImages["epitome"] = cvCreateImage( cvSize(_sourceImage->width, _sourceImage->height), IPL_DEPTH_8U, 1);

    foreach (Patch* match, _matches) {
        //        std::cout << match->_x << " " << match->_x  << std::endl;
        copy_block(_sourceImage, _debugImages["epitome"], cvRect(match->_x, match->_y, match->_w, match->_h ) );
    }

    return _debugImages["epitome"];
}


IplImage* SeedMap::meanIpl() {
    _debugImages["mean"] = cvCreateImage( cvSize(_width, _height), IPL_DEPTH_32F, 1);

    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){
            float mean = this->at(x,y)->histMean();
            cvSet2D( _debugImages["mean"] , y, x, cvScalarAll(mean) );
        }
    }
    return _debugImages["mean"] ;
}


IplImage* SeedMap::orientIpl(float delta) {
    _debugImages["orient"] = cvCreateImage( cvSize(_width, _height), IPL_DEPTH_32F, 1);
    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){

            float max_f = at(x,y)->_orientHist->peak();
            cvSet2D( _debugImages["orient"], y, x, cvScalarAll(delta-max_f) );
        }
    }
    return _debugImages["orient"];
}



void SeedMap::setImage(IplImage* image) {
    _sourceImage = image;

    IplImage* currentImage = image;

    int w = 16;
    int h = 16;

    foreach(IplImage* image, _debugImages) {
        if(!image) cvReleaseImage(&image);
    }

    this->_seeds.clear(); // remove all old patches

    float scale = 1.5f;
    float scaleWidth  = _sourceImage->width;
    float scaleHeight = _sourceImage->height;

    // TODO: create image scales :) 1.5, 1.5^2, 1.5^3

    for (int i=0; i<1; i++) {



        _width = (currentImage->width-w) / _xgrid;
        _height = (currentImage->height-h) / _ygrid;

        // generate new patches
        for(int y=0; y<_height; y++){
            for(int x=0; x<_width; x++){
                Patch* seed = new Patch( currentImage, x*_xgrid, y*_ygrid, w, h );
                seed->_scale = scale;
                this->_seeds.append(seed);
            }
        }

        scale *= 1.5f;
        scaleWidth  = _sourceImage->width / scale;
        scaleHeight = _sourceImage->height / scale;
        currentImage = cvCreateImage( cvSize(scaleWidth, scaleWidth), IPL_DEPTH_8U, 1);
        cvResize(_sourceImage, currentImage);

    }





}
