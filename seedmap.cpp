
#include <opencv/cv.h>
#include "seedmap.h"
#include "cv_ext.h"


SeedMap::SeedMap( IplImage* image, int w, int h, int xgrid, int ygrid )
    : _patchW(w), _patchH(h), _xgrid(xgrid), _ygrid(ygrid), _sourceImage(0)
{
    setImage(image);
}

void SeedMap::testPatch(int x, int y) {
    Patch* patch = new Patch( _sourceImage, x*_patchW, y*_patchH, _patchW, _patchH );
    this->match(*patch);
    free(patch);
}

void SeedMap::match(Patch &patch) {

    patch.findFeatures();

    QList<Transform*> localTransforms;
    Transform* t;
    foreach (Patch* seed, this->_seeds ) {
        t = patch.match(*seed, _error);
        if ( t ) {
//            _matches.append(seed);
              localTransforms.append(t);
//            break; // take first match
        }

    }
    _transforms.append(localTransforms.last());
    IplImage* reconstruction = reconstructIpl();
    _debugAlbum->fromIpl( reconstruction, "reconstruction" );
    cvReleaseImage(&reconstruction);


//  IplImage *warped = t->warp();

/*
    float va[][3] = { {t->_seed->_x,t->_seed->_y, 1},
                     {t->_seed->_x+patch._w,t->_seed->_y, 1},
                     {t->_seed->_x+patch._w,t->_seed->_y+patch._h, 1},
                     {t->_seed->_x,t->_seed->_y+patch._h, 1} };

    CvPoint points[4];
    for (int i=0; i<4; i++) {
        float tmp[] = { 0, 0 };
        CvMat bla = cvMat(3, 1, CV_32FC1,va[i]);
        CvMat result = cvMat(2, 1, CV_32FC1,tmp);
        cvMatMul(t->_warpMat, &bla, &result);
    }
*/
/*
    cvRectangle(warped, cvPoint(t->_seed->_x,t->_seed->_y), cvPoint(t->_seed->_x+patch._w,t->_seed->_y+patch._h),cvScalarAll(255));
    cvRectangle(warped, cvPoint(t->_x,t->_y), cvPoint(t->_x+patch._w,t->_y+patch._h),cvScalarAll(155));
    _debugAlbumR->fromIpl(warped, "warped");
*/
//    cvReleaseImage(&warped);

    _debugAlbum->updateGL();
    _debugAlbumR->updateGL();

}


Patch* SeedMap::at(int x, int y) {
    return this->_seeds.at(y*_width + x);
}


IplImage* SeedMap::reconstructIpl() {
    _debugImages["reconstuct"] = cvCreateImage( cvSize(_sourceImage->width, _sourceImage->height), IPL_DEPTH_8U, 1);
    cvZero(_debugImages["reconstuct"]);

    foreach (Transform* t, _transforms) {
        //        std::cout << t->_seedX << " " << t->_seedY << " " << t->_x << " "  << t->_y << std::endl;
        int w = t->_seed->_w;
        int h = t->_seed->_h;

        IplImage* reconstruction = t->reconstruct();

        copyBlock(reconstruction, _debugImages["reconstuct"] , cvRect(0, 0, w, h), cvRect(t->_x, t->_y, w, h) );
        cvReleaseImage(&reconstruction);
    }

    return _debugImages["reconstuct"];
}


IplImage* SeedMap::epitomeIpl() {
    _debugImages["epitome"] = cvCreateImage( cvSize(_sourceImage->width, _sourceImage->height), IPL_DEPTH_8U, 1);
    cvZero(_debugImages["epitome"]);

    foreach (Patch* match, _matches) {
        copyBlock(_sourceImage, _debugImages["epitome"], cvRect(match->_x, match->_y, match->_w, match->_h ) );
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

    int w = _patchW;
    int h = _patchH;

    foreach(IplImage* image, _debugImages) {
        if(!image) cvReleaseImage(&image);
    }

    this->_seeds.clear(); // remove all old patches

    float scale = 1.0f;
    float scaleWidth  = _sourceImage->width;
    float scaleHeight = _sourceImage->height;

    // TODO: create image scales :) 1.5, 1.5^2, 1.5^3

    for (int i=0; i<3; i++) {



        _width = (currentImage->width-w) / _xgrid;
        _height = (currentImage->height-h) / _ygrid;

        IplImage* flipped = cvCloneImage(currentImage);
        cvFlip( currentImage, flipped, 0);

        // generate new patches
        for(int y=0; y<_height; y++){
            for(int x=0; x<_width; x++){
                Patch* seed = new Patch( currentImage, x*_xgrid, y*_ygrid, w, h );
                seed->_scale = scale;
                this->_seeds.append(seed);

                seed = new Patch( flipped, x*_xgrid, y*_ygrid, w, h );
                seed->_scale = scale*-1.0;
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
