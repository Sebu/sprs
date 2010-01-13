
#include <opencv/highgui.h>
#include "seedmap.h"
#include "cv_ext.h"


SeedMap::SeedMap(cv::Mat& image, int w, int h, int xgrid, int ygrid )
    : patchW(w), patchH(h), xgrid(xgrid), ygrid(ygrid)
{
    setImage(image);
}

Patch* SeedMap::getPatch(int x, int y) {
    Patch* patch = this->patches[y * (sourceImage.cols/patchW) + x];
    return patch;
}

void SeedMap::saveReconstruction(std::string filename) {
    std::cout << filename << std::endl;
    cv::imwrite(filename, debugReconstruction());
}


void SeedMap::match(Patch& patch) {

    patch.findFeatures();

    if (!patch.matches) {

        patch.matches = new std::vector<Transform*>;

        Transform* transform = 0;
        for(uint i=0; i< seeds.size(); i++) {
            Patch* seed = seeds[i];

            transform = patch.match(*seed, maxError);
            if (transform) {
                patch.matches->push_back(transform);

                if (seed->isPatch())
                    seed->matches=patch.matches;

//                break; // take first match
            }
        }
    } else {
        std::cout << "shares matches" << std::endl;
    }

    
}


Patch* SeedMap::getSeed(int x, int y) {
    return this->seeds.at(y*width + x);
}


cv::Mat SeedMap::debugReconstruction() {
    debugImages["reconstuct"] = cv::Mat::zeros(sourceImage.size(), CV_8UC3);
    
    for(uint i=0; i< patches.size(); i++) {
        Patch* patch = patches[i];

        if (!patch->matches || patch->matches->empty()) continue;

        Transform* t = patch->matches->back();

        int w = t->seed->w_;
        int h = t->seed->h_;
        
        cv::Mat reconstruction(t->reconstruct());
        copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, w, h), cv::Rect(patch->x_, patch->y_, w, h) );
    }
    
    return debugImages["reconstuct"];
}


cv::Mat SeedMap::debugEpitome() {
    //    _debugImages["epitome"] = cvCreateImage( _sourceImage.size(), CV_8UC1);
    //    cvZero(_debugImages["epitome"]);
    
    /*    foreach (Patch* match, matches) {
        //        copyBlock(_sourceImage, _debugImages["epitome"], cvRect(match->_x, match->_y, match->_w, match->_h ) );
    }
  */
    return debugImages["epitome"];
}


cv::Mat SeedMap::debugMean() {
    debugImages["mean"] = cvCreateImage( cvSize(width, height), IPL_DEPTH_32F, 1);
    
    for(int y=0; y<height; y++){
        for(int x=0; x<width; x++){
            //            float mean = this->at(x,y)->getHistMean();
            //            cvSet2D( debugImages["mean"] , y, x, cvScalarAll(mean) );
        }
    }
    return debugImages["mean"];
}


cv::Mat SeedMap::debugOrientation(float delta) {
    debugImages["orient"] = cvCreateImage( cvSize(width, height), IPL_DEPTH_32F, 1);
    for(int y=0; y<height; y++){
        for(int x=0; x<width; x++){
            
            float max_f = getSeed(x,y)->orientHist->peak();
            //            cvSet2D( debugImages["orient"], y, x, cvScalarAll(delta-max_f) );
        }
    }
    return debugImages["orient"];
}



void SeedMap::setImage(cv::Mat& image) {
    sourceImage = image;
    
    cv::Mat currentImage = sourceImage;
    
    int w = patchW;
    int h = patchH;
    
    
    this->seeds.clear(); // remove all old patches
    
    float scale = 1.0f;
    float scaleWidth  = sourceImage.cols;
    float scaleHeight = sourceImage.rows;
    
    // TODO: create image scales :) 1.5, 1.5^2, 1.5^3
    
    for (int i=0; i<1; i++) {
        
        width = (currentImage.cols-w) / xgrid;
        height = (currentImage.rows-h) / ygrid;
        
        //        cv::Mat flipped = currentImage.clone();
        //        cv::flip(currentImage, flipped, 0);
        
        // generate new patches
        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){
                Patch* seed = new Patch( currentImage, x*xgrid, y*ygrid, w, h );
                seed->scale = scale;
                this->seeds.push_back(seed);
                if (seed->isPatch())
                    this->patches.push_back(seed);
                
                //                seed = new Patch( flipped, x*xgrid, y*ygrid, w, h );
                //                seed->scale = scale*-1.0;
                //                this->seeds.append(seed);
            }
        }
        
        
        //        scale *= 1.5f;
        //        scaleWidth  = _sourceImage.cols / scale;
        //        scaleHeight = _sourceImage.rows / scale;
        //        currentImage = cvCreateImage( cvSize(scaleWidth, scaleWidth), IPL_DEPTH_8U, _sourceImage->nChannels);
        //        cvResize(_sourceImage, currentImage);
        
    }
    
    
}
