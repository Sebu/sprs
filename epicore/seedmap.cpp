
#include <opencv/highgui.h>
#include <fstream>
#include <tr1/memory>
#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"


bool patchSorter (Patch* i, Patch* j) { return (i->overlapingBlocks.size() > j->overlapingBlocks.size() ); }

void SeedMap::generateEpitomes() {

    std::list<Patch*> sortedPatches;

    // crappy setup
    foreach(Patch* patch, patches) {
        sortedPatches.push_back(patch);
        bool blockFound = false;
        foreach(Match* m, *(patch->matches_)) {
            foreach(Patch* p, patches) {
                if ( m->hull_.intersect(p->hull_) ) {
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
        tmp.push_back(patch);

        sortedPatches.pop_front();

        Epitome* epi = new Epitome();

        // TODO: grow epitome :?
        // do {
        std::vector<Patch*> gotSatisfied;

        foreach(Match* m, patch->overlapingMatches) {

            // block of match allready satisfied?
            if(m->patch->satisfied_) continue;
            m->patch->satisfied_=true;
            gotSatisfied.push_back(m->patch);

            // FIXME: grow only block segments
            foreach(Patch* cover, m->overlapedSeeds) {
                if(!cover->satisfied_) {
                    cover->satisfied_ = true;
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
        int deltaE = (int)epi->reconPatches.size() * patchW_ * patchH_;

        int deltaI = (int)gotSatisfied.size() * patchW_ * patchH_;

        int benefit = deltaI  - deltaE;
        std::cout << "benefit: " << benefit << std::endl;

        //         } while(benefit(deltaE)>0)
        if(benefit>=0) {
            patch->satisfied_ = true;
            epitomes.push_back(epi);
            epi->save();
        } else {
            foreach(Patch* p, gotSatisfied)
                p->satisfied_ = false;

        }
    }
    Epitome* e = epitomes.front();
    cv::imshow("test",e->getMap());

}

void SeedMap::loadMatches(std::string fileName) {
    std::ifstream ifs( (fileName + ".txt").c_str() );

    if (ifs) {
        float tmp;
        ifs >> patchW_ >> xgrid_ >> tmp;
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

    ofs << patchW_ << " " << xgrid_ << " " << maxError << " ";
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
    Patch* patch = patches[y * (sourceImage.cols/patchW_) + x];
    return patch;
}

void SeedMap::saveReconstruction(std::string filename) {
    std::cout << filename << std::endl;
    cv::imwrite(filename, debugReconstruction());
}


void SeedMap::match(Patch& patch) {

    patch.findFeatures();

    if (!patch.matches_) {
        patch.matches_ = new std::vector<Match*>;

        #pragma omp parallel for
        for(uint i=0; i< seeds.size(); i++) {
            Patch* seed = seeds[i];
            if(!termCalculate) {
                Match* match = patch.match(*seed, maxError);
                if (match) {

                    #pragma omp critical
                    patch.matches_->push_back(match);

                    if (!match->transformed_ && seed->isPatch() && findAllMatches) {
                        int indexX = seed->x_ / patchW_;
                        int indexY = seed->y_ / patchH_;
                        Patch* p = getPatch(indexX, indexY);
                        if (!p->matches_) {
                           p->matches_=patch.matches_;
                           p->sharesMatches_ = true;
                        }
                    }

                    if (!findAllMatches) termCalculate=true;
                }
            }

        }
//        if(termCalculate)
//            patch.resetMatches();

    } else {
        std::cout << "shares " << patch.matches_->size() <<  " matches @ " <<  patch.x_/16 << " " << patch.y_/16 << std::endl;

        //copy matches and recalculate colorScale!
        patch.copyMatches();
    }

    
}



cv::Mat SeedMap::debugEpitomeMap() {
    cv::Mat image = debugImages["epitomemap"] = cv::Mat::zeros(sourceImage.size(), CV_8UC3);

    int color = 0;
    int step = 255/epitomes.size();
    foreach(Epitome* epi, epitomes) {
        foreach(Patch* patch, epi->reconPatches) {
            cv::rectangle(image, patch->hull_.verts[0], patch->hull_.verts[2],cv::Scalar((128-color) % 255,(255-color) % 255,color,255), 2);
        }
        foreach(Patch* patch, epi->reconPatches) {
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

        if (!patch->matches_ || patch->matches_->empty()) continue;


        Match* m = patch->matches_->front();
        cv::Mat reconstruction(m->reconstruct());
        copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, w, h), cv::Rect(patch->x_, patch->y_, w, h) );



        
    }
    
    return debugImages["reconstuct"];
}

void SeedMap::setImage(cv::Mat &image) {

    // add patches
    int width =  image.cols / patchW_;
    int height = image.rows / patchH_;
    for(int y=0; y<height; y++)
        for(int x=0; x<width; x++) {
            patches.push_back(new Patch( image, sourceGray, x*patchW_, y*patchH_, patchW_, patchH_,  1.0f, 0));
        }
}

void SeedMap::addSeedsFromImage(cv::Mat& source, int depth) {

    // add seeds
    float scale = 1.0f;
    for (int z=0; z< depth; z++) {

        float scaleWidth  = source.cols / scale;
        float scaleHeight = source.rows / scale;

        int width =  ((scaleWidth-patchW_) / xgrid_) + 1;
        int height = ((scaleHeight-patchH_) / ygrid_) + 1;

        // generate new patches
        for(int y=0; y<height; y++)
            for(int x=0; x<width; x++)
                for(int flip=0; flip<3; flip++)
                    seeds.push_back(new Patch( source, sourceGray, x*xgrid_, y*ygrid_, patchW_, patchH_,  scale, flip));

        scale *= 1.5f;
    }
}

void SeedMap::addSeedsFromEpitomes() {
    foreach(Epitome* epi, epitomes) {
        cv::Mat map = epi->getMap();
        addSeedsFromImage(map,1);
    }
}

void SeedMap::setReconSource(cv::Mat& image, int depth) {

    // remove all old seeds
    foreach(Patch* seed, seeds) {
        delete seed;
    }
    seeds.clear();

    // load epitomes?
    // add seeds from epitomes

    addSeedsFromImage(image, depth);

}

SeedMap::SeedMap(cv::Mat& image, cv::Mat& base, int s)
    : termCalculate(0), patchW_(s), patchH_(s), xgrid_(s/4), ygrid_(s/4), matchStep(0), maxError(0.0f)
{

    // add border to image
    int rightBorder =  patchW_ - (image.cols % patchW_);
    int bottomBorder = patchH_ - (image.rows % patchH_);
    sourceImage = cv::Mat::zeros(image.size()+cv::Size(rightBorder,bottomBorder), image.type());
    cv::Mat region(sourceImage, cv::Rect(cv::Point(0,0),image.size()));
    image.copyTo(region);


    rightBorder =  patchW_ - (base.cols % patchW_);
    bottomBorder = patchH_ - (base.rows % patchH_);
    baseImage = cv::Mat::zeros(base.size()+cv::Size(rightBorder,bottomBorder), base.type());
    cv::Mat baseRegion(baseImage, cv::Rect(cv::Point(0,0),base.size()));
    base.copyTo(baseRegion);


    // create gray version
    cv::cvtColor(sourceImage, sourceGray, CV_BGR2GRAY);
    setImage(sourceImage);
    setReconSource(baseImage, 3);

}
