
#include <opencv/cv.h>
#include "seedmap.h"
#include "cv_ext.h"


SeedMap::SeedMap(cv::Mat& image, int w, int h, int xgrid, int ygrid )
    : _patchW(w), _patchH(h), _xgrid(xgrid), _ygrid(ygrid)
{
    setImage(image);
}

void SeedMap::testPatch(int x, int y) {

    Patch* patch = new Patch( sourceImage, x*_patchW, y*_patchH, _patchW, _patchH );
    this->match(*patch);
    delete patch;

}

void SeedMap::match(Patch &patch) {


    patch.findFeatures();

    Transform* transform = 0;
    foreach (Patch* seed, this->_seeds ) {
        transform = patch.match(*seed, _error);
        if ( transform ) {
            patch.matches.append(transform);
            //            break; // take first match
        }
    }
    if (!patch.matches.isEmpty()) {
        transforms.append(patch.matches.first());

        cv::Mat warped = this->sourceImage.clone(); //transform->warp();
        cv::Mat warpM = cv::Mat::eye(3,3,CV_64FC1);
        cv::Mat selection( warpM, cv::Rect(0,0,3,2) );
        cv::Mat rotM;
        // highlight block
        cv::rectangle(warped, cv::Point(patch._x, patch._y),
                      cv::Point(patch._x+patch._w, patch._y+patch._h),
                      cv::Scalar(0,255,0,100),2);

        foreach (Transform* current, patch.matches) {

            double points[4][2] = { {current->seed->_x,    current->seed->_y},
                                    {current->seed->_x+16, current->seed->_y},
                                    {current->seed->_x+16, current->seed->_y+16},
                                    {current->seed->_x,    current->seed->_y+16}
            };

            cv::Point newPoints[4];

            invertAffineTransform(current->warpMat, selection);
            invertAffineTransform(current->rotMat, rotM);

            for(int i=0; i<4; i++ ) {
                cv::Mat p = (cv::Mat_<double>(3,1) << points[i][0], points[i][1], 1);

                cv::Mat a =  rotM * (warpM * p);

                newPoints[i].x = a.at<double>(0,0);
                newPoints[i].y = a.at<double>(0,1);

            }
            for(int i=0; i<4; i++){
                // highlight match
                cv::line(warped, newPoints[i], newPoints[(i+1) % 4], cv::Scalar(0,0,255,100));

            }

        }
        _debugAlbumR->fromIpl( warped, "preview" );
    }

    cv::Mat reconstruction(reconstructIpl());
    _debugAlbum->fromIpl( reconstruction, "reconstruction" );

    _debugAlbum->updateGL();
    _debugAlbumR->updateGL();
    
}


Patch* SeedMap::at(int x, int y) {
    return this->_seeds.at(y*_width + x);
}


cv::Mat SeedMap::reconstructIpl() {
    debugImages["reconstuct"] = cv::Mat::zeros(sourceImage.size(), CV_8UC3);
    
    foreach (Transform* t, transforms) {
        int w = t->seed->_w;
        int h = t->seed->_h;
        
        cv::Mat reconstruction(t->reconstruct());
        copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, w, h), cv::Rect(t->_x, t->_y, w, h) );
    }
    
    return debugImages["reconstuct"];
}


cv::Mat SeedMap::epitomeIpl() {
    //    _debugImages["epitome"] = cvCreateImage( _sourceImage.size(), CV_8UC1);
    //    cvZero(_debugImages["epitome"]);
    
    foreach (Patch* match, _matches) {
        //        copyBlock(_sourceImage, _debugImages["epitome"], cvRect(match->_x, match->_y, match->_w, match->_h ) );
    }
    
    return debugImages["epitome"];
}


cv::Mat SeedMap::meanIpl() {
    debugImages["mean"] = cvCreateImage( cvSize(_width, _height), IPL_DEPTH_32F, 1);
    
    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){
            //            float mean = this->at(x,y)->getHistMean();
            //            cvSet2D( debugImages["mean"] , y, x, cvScalarAll(mean) );
        }
    }
    return debugImages["mean"] ;
}


cv::Mat SeedMap::orientIpl(float delta) {
    debugImages["orient"] = cvCreateImage( cvSize(_width, _height), IPL_DEPTH_32F, 1);
    for(int y=0; y<_height; y++){
        for(int x=0; x<_width; x++){
            
            float max_f = at(x,y)->orientHist->peak();
            //            cvSet2D( debugImages["orient"], y, x, cvScalarAll(delta-max_f) );
        }
    }
    return debugImages["orient"];
}



void SeedMap::setImage(cv::Mat& image) {
    sourceImage = image;
    
    cv::Mat currentImage = sourceImage;
    
    int w = _patchW;
    int h = _patchH;
    
    
    this->_seeds.clear(); // remove all old patches
    
    float scale = 1.0f;
    float scaleWidth  = sourceImage.cols;
    float scaleHeight = sourceImage.rows;
    
    // TODO: create image scales :) 1.5, 1.5^2, 1.5^3
    
    for (int i=0; i<1; i++) {
        
        _width = (currentImage.cols-w) / _xgrid;
        _height = (currentImage.rows-h) / _ygrid;
        
//        cv::Mat flipped = currentImage.clone();
//        cv::flip(currentImage, flipped, 0);
        
        // generate new patches
        for(int y=0; y<_height; y++){
            for(int x=0; x<_width; x++){
                Patch* seed = new Patch( currentImage, x*_xgrid, y*_ygrid, w, h );
                seed->scale = scale;
                this->_seeds.append(seed);
                
//                seed = new Patch( flipped, x*_xgrid, y*_ygrid, w, h );
//                seed->scale = scale*-1.0;
//                this->_seeds.append(seed);
            }
        }
        
        
        //        scale *= 1.5f;
        //        scaleWidth  = _sourceImage.cols / scale;
        //        scaleHeight = _sourceImage.rows / scale;
        //        currentImage = cvCreateImage( cvSize(scaleWidth, scaleWidth), IPL_DEPTH_8U, _sourceImage->nChannels);
        //        cvResize(_sourceImage, currentImage);
        
    }
    
    
}
