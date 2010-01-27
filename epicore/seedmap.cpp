
#include <opencv/highgui.h>
#include <fstream>
#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"

SeedMap::SeedMap(cv::Mat& image, int w, int h, int xgrid, int ygrid )
    : termCalculate(0), patchW(w), patchH(h), xgrid(xgrid), ygrid(ygrid), matchStep(0)
{

    setImage(image,3);
}

bool myfunction (Patch* i, Patch* j) { return (i->overlapedMatches.size() > j->overlapedMatches.size() ); }

void SeedMap::generateEpitomes() {

    std::list<Patch*> sortedPatches;
    std::vector<Patch*> satisfied;


    // crappy setup
    for(uint i=0; i<patches.size(); i++) {
        Patch* patch=patches[i];
        sortedPatches.push_back(patch);
        for(uint j=0; j<patch->matches->size(); j++) {
            Match* m = patch->matches->at(j);
            Polygon poly = m->getMatchbox();
            for(int k=0; k<patches.size(); k++) {
                Patch* p = patches[k];
                if ( poly.intersect(p->hull) ) {
                    m->overlapedPatches.push_back(p);
                    p->overlapedMatches.push_back(m);
                }

            }
        }
    }

    // sort patches by overlap count
    sortedPatches.sort(myfunction);

    // create epitomes
    while(sortedPatches.size()!=0) {

        std::list<Patch*> tmp;
        Patch* patch = sortedPatches.front();
        sortedPatches.pop_front();

        Epitome* epi = new Epitome();

        for(uint i=0; i < patch->overlapedMatches.size(); i++) {
            Match* m = patch->overlapedMatches.at(i);
            std::vector<Patch*>::iterator it=std::find(satisfied.begin(),satisfied.end(),m->patch);
            if (satisfied.size()!=0 && (it!=satisfied.end() || (*it)==m->patch)) continue;
            satisfied.push_back(m->patch);
            tmp.push_back(patch);
            foreach(Patch* over, m->overlapedPatches) {
//                satisfied.push_back(over);
                tmp.push_back(over);
            }
        }

        while(tmp.size()!=0){
            Patch* p = tmp.front();
            epi->reconPatches.push_back(p);
            tmp.remove(p);
        }
        epitomes.push_back(epi);
    }

}

void SeedMap::loadMatches(std::string fileName) {
    std::ifstream ifs( (fileName + ".txt").c_str() );

    if (ifs) {
        std::string version;
        ifs >> version;
        ifs.ignore(8192, '\n');
        ifs >> fileName;
        ifs.ignore(8192, '\n');

        ifs >> patchW >> xgrid >> maxError;
        ifs.ignore(8192, '\n');
        int size;
        ifs >> size;
        ifs.ignore(8192, '\n');

        for(int i=0; i<size; i++)
            patches[i]->deserialize(ifs);
    }

    ifs.close();
}

// filename
//
void SeedMap::saveMatches(std::string fileName) {
    std::ofstream ofs( (fileName + ".txt").c_str() );

    ofs << "version 1.0" << std::endl;
    ofs << fileName << std::endl;
    ofs << patchW << " " << xgrid << " " << maxError << std::endl;
    ofs << patches.size() << std::endl;
    for(uint i=0; i<patches.size(); i++)
        patches[i]->serialize(ofs);

    ofs.close();
}

void SeedMap::resetMatches() {
    matchStep=0;
    for(uint i=0; i<patches.size(); i++) {
        Patch* patch = patches[i];
        if(patch->matches) {
            patches[i]->matches->clear();
            delete patch->matches;
            patch->matches = 0;
        }
    }
}

Patch* SeedMap::matchNext() {
    if (matchStep>=patches.size()) return 0;
    Patch* patch = patches[matchStep++];
    match(*patch);
    return patch;
}

Patch* SeedMap::getPatch(int x, int y) {
    Patch* patch = patches[y * (sourceImage.cols/patchW) + x];
    return patch;
}

void SeedMap::saveReconstruction(std::string filename) {
    std::cout << filename << std::endl;
    cv::imwrite(filename, debugReconstruction());
}


void SeedMap::match(Patch& patch) {

    patch.findFeatures();

    if (!patch.matches) {
        patch.matches = new std::vector<Match*>;

#pragma omp parallel for
        for(uint i=0; i< seeds.size(); i++) {
            if(!termCalculate) {
                Match* match = 0;
                Patch* seed = seeds[i];
                seed->transformed = false; // TODO: reset is better or move flag to match

                match = patch.match(*seed, maxError);
                if (match) {
                    match->patch = &patch;

#pragma omp critical
                    patch.matches->push_back(match);

                    if (seed->isPatch())
                        seed->matches=patch.matches;
                    // break; // take first match
                }
            }

        }
        if(termCalculate) {
            patch.matches->clear();
            delete (patch.matches);
            patch.matches = 0;

        }
    } else {
        std::cout << "shares " << patch.matches->size() <<  " matches @ " <<  patch.x_/16 << " " << patch.y_/16 << std::endl;
    }

    
}


Patch* SeedMap::getSeed(int x, int y) {
    return this->seeds.at(y*width + x);
}

cv::Mat SeedMap::debugEpitomeMap() {
    cv::Mat image = debugImages["epitomemap"] = cv::Mat::zeros(sourceImage.size(), CV_8UC3);

    int color = 0;
    int step = 255/epitomes.size();
    foreach(Epitome* epi, epitomes) {
        foreach(Patch* patch, epi->reconPatches) {
//            copyBlock(patch->patchImage, debugImages["epitomemap"], cv::Rect(0, 0, patch->w_, patch->h_), cv::Rect(patch->x_, patch->y_, patch->w_, patch->h_) );
            cv::rectangle(image, patch->hull.verts[0], patch->hull.verts[2],cv::Scalar(color,255-color,color,255), CV_FILLED);
        }
        color += step;
    }


    return debugImages["epitomemap"];
}

cv::Mat SeedMap::debugReconstruction() {
    debugImages["reconstuct"] = cv::Mat::zeros(sourceImage.size(), CV_8UC3);
    
    for(uint i=0; i< patches.size(); i++) {
        Patch* patch = patches[i];
        int w = patch->w_;
        int h = patch->h_;

        if (!patch->matches || patch->matches->empty()) continue;


        Match* m = patch->matches->front();
        cv::Mat reconstruction(m->reconstruct());
        copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, w, h), cv::Rect(patch->x_, patch->y_, w, h) );



        
    }
    
    return debugImages["reconstuct"];
}



void SeedMap::setImage(cv::Mat& image, int depth) {

    int w = patchW;
    int h = patchH;

    int rightBorder =  w - (image.cols % w);
    int bottomBorder = h - (image.rows % h);

    // add border to image
    sourceImage = cv::Mat::zeros(image.size()+cv::Size(rightBorder,bottomBorder), image.type());
    cv::Mat region(sourceImage, cv::Rect(cv::Point(0,0),image.size()));
    sourceImage = image; //image.copyTo(region);

    cv::Mat currentImage = sourceImage;
    
    
    this->seeds.clear(); // remove all old patches
    
    float scale = 1.0f;
    float scaleWidth  = sourceImage.cols;
    float scaleHeight = sourceImage.rows;
    


    for (int i=0; i< depth; i++) {
        
        width =  ((scaleWidth-w) / xgrid); //+ 1;
        height = ((scaleHeight-h) / ygrid); // + 1;

        cv::Mat flipped = currentImage.clone();
        cv::flip(currentImage, flipped, 0);
        
        // generate new patches
        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){


                Patch* seed = new Patch( currentImage, x*xgrid, y*ygrid, w, h );
                seed->scale = scale;
                this->seeds.push_back(seed);
                if (seed->isPatch())
                    this->patches.push_back(seed);
                
//                seed = new Patch( flipped, x*xgrid, y*ygrid, w, h );
//                seed->scale = scale;
//                this->seeds.push_back(seed);

            }
        }
        

        scale *= 1.5f;
        scaleWidth  = sourceImage.cols / scale;
        scaleHeight = sourceImage.rows / scale;
//        cv::resize(sourceImage, currentImage, cv::Size(scaleWidth, scaleHeight) );
        
    }
    
    
}
