
#include <opencv/highgui.h>
#include <fstream>
#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"

SeedMap::SeedMap(cv::Mat& image, int w, int h, int xgrid, int ygrid )
    : termCalculate(0), patchW(w), patchH(h), xgrid(xgrid), ygrid(ygrid), matchStep(0), maxError(0.0f)
{

    setImage(image,3);
}

bool patchSorter (Patch* i, Patch* j) { return (i->overlapingBlocks.size() > j->overlapingBlocks.size() ); }

void SeedMap::generateEpitomes() {

    std::list<Patch*> sortedPatches;


    // crappy setup
    foreach(Patch* patch, patches) {
        sortedPatches.push_back(patch);
        bool blockFound = false;
        foreach(Match* m, *(patch->matches)) {
            Polygon matchHull = m->getMatchbox();
            foreach(Patch* p, patches) {
                if ( matchHull.intersect(p->hull) ) {
                    m->overlapedSeeds.push_back(p);
                    p->overlapingMatches.push_back(m);
                    if(!blockFound)
                        p->overlapingBlocks.push_back(patch);
                    blockFound=true;
                }
            }
        }
    }
    std::cout <<  "pre calc done"  << std::endl;


    // sort patches by overlap count
    sortedPatches.sort(patchSorter);

    // create epitomes
    while(sortedPatches.size()!=0) {

        std::list<Patch*> tmp;
        Patch* patch = sortedPatches.front();
        sortedPatches.pop_front();
        tmp.push_back(patch);


        Epitome* epi = new Epitome();

        // TODO: grow epitome :?
        // do {
        std::vector<Patch*> gotSatisfied;

        foreach(Match* m, patch->overlapingMatches) {

            // block of match allready satisfied?
            if(m->patch->satisfied) continue;
            m->patch->satisfied=true;
            gotSatisfied.push_back(m->patch);

            // FIXME: grow only block segments
            foreach(Patch* cover, m->overlapedSeeds) {
                if(!cover->satisfied) {
                    cover->satisfied = true;
                    gotSatisfied.push_back(cover);
                }
                tmp.push_back(cover);
            }
        }

        // detlaE ??
        while(tmp.size()!=0) {
            Patch* p = tmp.front();
            epi->reconPatches.push_back(p);
            tmp.remove(p);
        }
        int deltaE = (int)epi->reconPatches.size() * patchW * patchH;

        int deltaI = (int)gotSatisfied.size() * patchW * patchH;

        int benefit = deltaI  - deltaE;
        std::cout << benefit << std::endl;

        // } while(benefit(deltaE)>0)
        if(benefit>=0)
            epitomes.push_back(epi);
        else {
            foreach(Patch* p, gotSatisfied)
                p->satisfied = false;

        }
    }

}

void SeedMap::loadMatches(std::string fileName) {
    std::ifstream ifs( (fileName + ".txt").c_str() );

    if (ifs) {
        float tmp;
        ifs >> patchW >> xgrid >> tmp;
        int size;
        ifs >> size;
        for(int i=0; i<size; i++)
            patches[i]->deserialize(ifs);
    }

    ifs.close();
}

// filename
//
void SeedMap::saveMatches(std::string fileName) {
    std::ofstream ofs( (fileName + ".txt").c_str() );

    ofs << patchW << " " << xgrid << " " << maxError << " ";
    ofs << patches.size() << " ";
    for(uint i=0; i<patches.size(); i++)
        patches[i]->serialize(ofs);

    ofs.close();
}

void SeedMap::resetMatches() {
    matchStep=0;
    for(uint i=0; i<patches.size(); i++)
        patches[i]->resetMatches();
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

                    if (seed->isPatch() && !(seed->matches)) {
                        seed->matches=patch.matches;
                        seed->sharesMatches = true;
                    }
                }
            }

        }
        if(termCalculate)
            patch.resetMatches();

    } else {
        std::cout << "shares " << patch.matches->size() <<  " matches @ " <<  patch.x_/16 << " " << patch.y_/16 << std::endl;
        //TODO: copy matches and recalculate colorScale!
        patch.copyMatches();
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
            cv::rectangle(image, patch->hull.verts[0], patch->hull.verts[2],cv::Scalar(color,255-color,color,255), 2);
            copyBlock(patch->patchImage, debugImages["epitomemap"], cv::Rect(0, 0, patch->w_, patch->h_), cv::Rect(patch->x_, patch->y_, patch->w_, patch->h_) );
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

    // create gray version
    cv::cvtColor(sourceImage, sourceGray, CV_BGR2GRAY);

    cv::Mat currentImage = sourceImage;
    
    
    this->seeds.clear(); // remove all old patches
    
    float scale = 1.0f;
    float scaleWidth  = sourceImage.cols;
    float scaleHeight = sourceImage.rows;
    

    for (int i=0; i< depth; i++) {
        
        width =  ((scaleWidth-w) / xgrid); //+ 1;
        height = ((scaleHeight-h) / ygrid); // + 1;

        Patch* seed=0;
        // generate new patches
        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){
                seed = new Patch( currentImage, sourceGray, x*xgrid, y*ygrid, w, h,  scale, 0);
                this->seeds.push_back(seed);
                if (seed->isPatch())
                    this->patches.push_back(seed);

                // fliped patches
                //*
                seed = new Patch( currentImage, sourceGray, x*xgrid, y*ygrid, w, h,  scale, 1);
                this->seeds.push_back(seed);
                seed = new Patch( currentImage, sourceGray, x*xgrid, y*ygrid, w, h,  scale, -1);
                this->seeds.push_back(seed);
                //*/
            }
        }
        

        scale *= 1.5f;
        scaleWidth  = sourceImage.cols / scale;
        scaleHeight = sourceImage.rows / scale;
        
    }
    
    
}
