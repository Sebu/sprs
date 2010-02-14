
#include <opencv/highgui.h>
#include <fstream>
#include <tr1/memory>
#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"


bool squareSorter (Square* i, Square* j) { return (i->overlapingMatches_.size() > j->overlapingMatches_.size() ); }


void SeedMap::generateEpitomes() {


    // create polyomino table

    std::vector<Square*> map;
    std::list<Square*> sortedSquares;

    uint width = baseImage.cols/4;
    uint height = baseImage.rows/4;


    for(uint y=0; y<height; y++) {
        for(uint x=0; x<width; x++) {
            Square* s = new Square(x*4,y*4);
            map.push_back(s);
            sortedSquares.push_back(s);
        }
    }


    // crappy setup
    foreach(Patch* block, blocks) {
        foreach(Match* match, *(block->matches_)) {
            // calc bbox of match
            AABB box = match->hull_.getBox();
            uint minx = box.min.m_v[0] / xgrid_;
            uint miny = box.min.m_v[1] / ygrid_;
            uint maxx = box.max.m_v[0] / xgrid_;
            uint maxy = box.max.m_v[1] / ygrid_;

            // find all potential seeds
            for(uint y=miny; y<maxy; y++) {
                for(uint x=minx; x<maxx; x++) {
                    Square* square = map.at(y*width + x);
                    // get only overlaped seeds
                    if (square->hull_.intersects(match->hull_)) {
                        match->overlapedSquares_.push_back(square);
                        square->overlapingMatches_.push_back(match);
                    }
                }
            }

        }
    }

    sortedSquares.sort(squareSorter);

    foreach(Square* s, sortedSquares)
        std::cout << s->overlapingMatches_.size() << " ";

    std::cout <<  "pre calc done"  << std::endl;

/*
    // sort patches by overlap count
    while(sortedSquares.size()!=0) {

        int deltaE = 0;
        int deltaI = 0;
        int benefit = 0;

        // get first block
        Square* square = sortedSquares.front();
        sortedSquares.pop_front();

        if(square->inUse_)
            continue;

        Epitome* epi = new Epitome();


        foreach(Match* match, square->overlapingMatches_) {

            // block of match allready satisfied?
            if(match->block->satisfied_) continue;
            benefit += epi->grow(match);

            match->block->satisfied_ = true;

        }

        std::cout << "benefit: " << benefit << std::endl;
    }
//*/

}

void SeedMap::loadMatches(std::string fileName) {
    std::ifstream ifs( (fileName + ".txt").c_str() );

    if (ifs) {
        float tmp;
        ifs >> patchW_ >> xgrid_ >> tmp;
        int size;
        ifs >> size;
        for(int i=0; i<size; i++)
            blocks[i]->deserialize(ifs);
    }

    ifs.close();
}

// filename
//
void SeedMap::saveMatches(std::string fileName) {
    std::ofstream ofs( (fileName + ".txt").c_str() );

    ofs << patchW_ << " " << xgrid_ << " " << maxError << " ";
    ofs << blocks.size() << " ";
    for(uint i=0; i<blocks.size(); i++)
        blocks[i]->serialize(ofs);

    ofs.close();
}

void SeedMap::resetMatches() {
    matchStep=0;
    for(uint i=0; i<blocks.size(); i++)
        blocks[i]->resetMatches();
}

Patch* SeedMap::matchNext() {
    if (matchStep>=blocks.size()) return 0;
    Patch* patch = blocks[matchStep++];
    match(*patch);
    return patch;
}

Patch* SeedMap::getPatch(int x, int y) {
    Patch* patch = blocks[y * (sourceImage.cols/patchW_) + x];
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

                    if (!match->transformed_ && seed->isPatch_ && findAllMatches) {
                        if (!seed->matches_) {
                            seed->matches_=patch.matches_;
                            seed->sharesMatches_ = true;
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
        foreach(Patch* patch, epi->reconSeeds_) {
            cv::rectangle(image, patch->hull_.verts[0], patch->hull_.verts[2],cv::Scalar((128-color) % 255,(255-color) % 255,color,255), 2);
        }
        foreach(Patch* patch, epi->reconSeeds_) {
            copyBlock(patch->patchImage, debugImages["epitomemap"], cv::Rect(0, 0, patch->w_, patch->h_), cv::Rect(patch->x_, patch->y_, patch->w_, patch->h_) );
        }

        color += step;
    }


    return debugImages["epitomemap"];
}

cv::Mat SeedMap::debugReconstruction() {
    debugImages["reconstuct"] = cv::Mat::zeros(sourceImage.size(), CV_8UC3);
    
    for(uint i=0; i< blocks.size(); i++) {
        Patch* patch = blocks[i];
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
        Patch* block = new Patch( image, sourceGray, x*patchW_, y*patchH_, patchW_, patchH_,  1.0f, 0);
        block->isPatch_ = true;
        blocks.push_back(block);
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

        // generate new seeds
        Patch* seed = 0;
        for(int y=0; y<height; y++) {
            int localY = y*ygrid_;
            for(int x=0; x<width; x++) {
                int localX = x*xgrid_;
                for(int flip=0; flip<3; flip++) {

                    if ( (localX % patchW_)==0 && (localY % patchH_)==0 && flip==0 && z==0 ) {
                        int indexX = localX / patchW_;
                        int indexY = localY / patchH_;
                        seed = getPatch(indexX, indexY);
                    }
                    else
                        seed = new Patch( source, sourceGray, localX, localY, patchW_, patchH_,  scale, flip);

                    seeds.push_back(seed);
                }
            }
        }



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
