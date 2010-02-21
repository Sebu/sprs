
#include <opencv/highgui.h>
#include <fstream>
#include <tr1/memory>
#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"


bool tileSorter (Tile* i, Tile* j) { return (i->blocks_ > j->blocks_ ); }


void SeedMap::generateEpitomes() {

    if(termCalculate_) return;

    std::list<Tile*> sortedTiles;
    uint width = baseImage_.cols/4;
    uint height = baseImage_.rows/4;


    // create tile table
    for(uint y=0; y<height; y++) {
        for(uint x=0; x<width; x++) {
            Tile* tile = new Tile(x*4,y*4);

            tiles_.push_back(tile);
            sortedTiles.push_back(tile);
        }
    }

    std::cout <<  "create squares done"  << std::endl;


    // crappy setup
    // find all potential seeds

    foreach(Patch* block, blocks_) {
        if(!block->matches_) continue;

        foreach(Match* match, *(block->matches_)) {
            // calc bbox of match
            AABB box = match->hull_.getBox();
            uint minx = std::max( (int)(box.min.m_v[0] / grid_), 0 );
            uint miny = std::max( (int)(box.min.m_v[1] / grid_), 0 );
            uint maxx = std::min( (int)(box.max.m_v[0] / grid_), (int)width-1);
            uint maxy = std::min( (int)(box.max.m_v[1] / grid_), (int)height-1);


            for(uint y=miny; y<=maxy; y++) {
                for(uint x=minx; x<=maxx; x++) {
                    Tile* tile = tiles_.at(y*width + x);

                    if (tile->hull_.intersects(match->hull_)) {
                        match->coveredTiles_.push_back(tile);
                        tile->overlapingMatches_.push_back(match);
                        tile->overlapingBlocks_.push_back(block);
                    }
                }
            }

        }
    }

    std::cout <<  "find overlapsed done"  << std::endl;

    // neighbours
    // FIXME: :D
    for(uint y=0; y<height; y++) {
        for(uint x=0; x<width; x++) {
            Tile* tile = tiles_[y*width+x];

            tile->overlapingBlocks_.unique();
            tile->blocks_ = tile->overlapingBlocks_.size();


            /*
            if (x>0 && y>0) {
                Tile* n = tiles_[(y-1)*width +(x-1)];
                tile->neighbours_.push_back(n);
            }

            if (x>0 && y<height-1) {
                Tile* n = tiles_[(y+1)*width +(x-1)];
                tile->neighbours_.push_back(n);
            }

            if (x<width-1 && y>0) {
                Tile* n = tiles_[(y-1)*width +(x+1)];
                tile->neighbours_.push_back(n);
            }
            if(x<width-1 && y<height-1) {
                Tile* n = tiles_[(y+1)*width +(x+1)];
                tile->neighbours_.push_back(n);
            }
            //*/
            if (x>0) {
                Tile* n = tiles_[y*width +(x-1)];
                tile->neighbours_.push_back(n);
            }
            if (y>0) {
                Tile* n = tiles_[(y-1)*width +x];
                tile->neighbours_.push_back(n);
            }
            if(x<width-1) {
                Tile* n = tiles_[y*width +(x+1)];
                tile->neighbours_.push_back(n);
            }
            if(y<height-1) {
                Tile* n = tiles_[(y+1)*width +x];
                tile->neighbours_.push_back(n);
            }

        }
    }
    std::cout <<  "create neighbours done"  << std::endl;

    sortedTiles.sort(tileSorter);

    std::cout <<  "pre calc done"  << std::endl;
    // reset satisfied



    // sort patches by overlap count
    while(!sortedTiles.empty()) {


        // get first block
        Tile* chartSquare = sortedTiles.front();
        sortedTiles.pop_front();

        if(chartSquare->done_)
            continue;

        Chart* chart = new Chart(&baseImage_);
        std::list<Tile*> chartTiles;

        chartTiles.push_back(chartSquare);
        // block coverrage
        while(!chartTiles.empty()) {
            std::vector<Patch*> deltaIBlocks;
            std::vector<Tile*> deltaETiles;
            int deltaE = 0;
            int deltaI = 0;

            Tile* current = chartTiles.front();
            chartTiles.pop_front();
            if(current->done_) continue;

            foreach(Match* match, current->overlapingMatches_) {

                // block of match allready satisfied?
                if(match->block_->satisfied_) continue;
                match->block_->satisfied_ = true;
                deltaIBlocks.push_back(match->block_);
                deltaI += 16*16;

                foreach(Tile* s, match->coveredTiles_) {
                    if(s->inUse_) continue;

                    s->inUse_=true;
                    deltaE += 16;
                    deltaETiles.push_back(s);
                }
            }
            int benefit = deltaI - deltaE;
            std::cout << "benefit: " << benefit << std::endl;
            if(benefit>=0) {
                // mark as all matches satisfied
                current->done_ = true;
                // add neighbours to chartTiles
                foreach(Tile* s, deltaETiles) {
                    chart->reconTiles_.push_back(s);
                    if(!s->done_)
                        chartTiles.push_back(s);
                    foreach(Tile* neighbour, s->neighbours_) {
                        if(!neighbour->done_)
                            chartTiles.push_back(neighbour);
                    }
                }


            } else {
                foreach(Tile* s, deltaETiles) s->inUse_ = false;
                foreach(Patch* b, deltaIBlocks) b->satisfied_ = false;
            }
        }

        if(!chart->reconTiles_.empty())
            charts_.push_back(chart);
        //        else
        //            if(!chartTile->done_)
        //                sortedTiles.push_back(chartTiles);

    }

    foreach(Patch* block, blocks_) {
        if(!block->matches_) continue;
        std::sort(block->matches_->begin(),block->matches_->end(),matchSorter);
        foreach(Match* match, *block->matches_) {
            bool covered = true;
            foreach(Tile* tile, match->coveredTiles_) {
                if(!tile->inUse_) { covered = false; break; }
            }
            if(covered) {
                block->finalMatch_ = match;
                break;
            }
        }
    }

    //*
    foreach(Tile* tile, tiles_) {
        tile->inUse_ = false;
    }

    foreach(Patch* block, blocks_) {
        if(block->finalMatch_) {
            foreach(Tile* tile, block->finalMatch_->coveredTiles_)
                tile->inUse_ = true;
        }
    }
    //*/

    foreach(Patch* p, blocks_)
        if(!p->satisfied_) std::cout << p->x_/16 << " " << p->y_/16 << std::endl;


}


//
void SeedMap::saveEpitome(std::string fileName) {
    cv::imwrite((fileName + ".epi.png").c_str(), debugEpitomeMap());

    std::ofstream ofs( (fileName + ".epi.txt").c_str() );
    foreach(Chart* c, charts_)
        foreach(Tile* t,c->reconTiles_)
            ofs << t->x_ << " " << t->y_ << " ";
    ofs.close();
}

// filename
//
void SeedMap::save(std::string fileName) {
    std::ofstream ofs( (fileName).c_str() );

    for(uint i=0; i<blocks_.size(); i++)
        blocks_[i]->save(ofs);
    ofs.close();
}


void SeedMap::deserialize(std::string fileName) {
    if (!searchInOriginal_) return;

    std::ifstream ifs( (fileName + ".cache").c_str() );

    if (ifs) {
        float error;
        ifs >> patchS_ >> grid_ >> error;
        if (maxError_ == error) {
            int size;
            ifs >> size;
            for(int i=0; i<size; i++)
                blocks_[i]->deserialize(ifs);
        }
    }

    ifs.close();
}


void SeedMap::serialize(std::string fileName) {
    std::ofstream ofs( (fileName + ".cache").c_str() );

    ofs << patchS_ << " " << grid_ << " " << maxError_ << " ";
    ofs << blocks_.size() << " ";
    for(uint i=0; i<blocks_.size(); i++)
        blocks_[i]->serialize(ofs);

    ofs.close();
}

void SeedMap::resetMatches() {
    matchStep_=0;
//    for(uint i=0; i<blocks_.size(); i++)
//        blocks_[i]->resetMatches();
}

void SeedMap::matchAll() {
    while(!termCalculate_ && matchNext()) {}
}

Patch* SeedMap::matchNext() {
    if (matchStep_>=blocks_.size()) return 0;
    Patch* patch = blocks_[matchStep_++];
    match(patch);
    return patch;
}

Patch* SeedMap::getPatch(int x, int y) {
    Patch* patch = blocks_[y * (sourceImage_.cols/patchS_) + x];
    return patch;
}

void SeedMap::saveReconstruction(std::string filename) {
    cv::imwrite(filename, debugReconstruction());
}


void SeedMap::match(Patch* patch) {

    patch->findFeatures();

    if (!patch->matches_) {
        patch->matches_ = new std::vector<Match*>;


        bool breakIt = false;
#pragma omp parallel for
        for(uint i=0; i< seeds_.size(); i++) {
            Patch* seed = seeds_[i];
            if(!termCalculate_ && !breakIt) {
                Match* match = patch->match(*seed, maxError_);
                if (match) {

#pragma omp critical
                    patch->matches_->push_back(match);

                    if (!match->transformed_ && seed->isBlock_ && searchInOriginal_) {
                        if (!seed->matches_) {
                            seed->matches_=patch->matches_;
                            seed->sharesMatches_ = true;
                        }
                    }

                    if (!searchInOriginal_) breakIt=true;
                }
            }

        }

        // add self match :P
        if(searchInOriginal_) {
            Match* selfMatch = new Match(patch);
            selfMatch->block_ = patch;
            selfMatch->transformed_ = patch->transformed_;
            selfMatch->error_ = 0.0;
            selfMatch->calcTransform();
            selfMatch->calcHull();
            patch->matches_->push_back(selfMatch);
        } else if (patch->matches_->empty()) {
          patch->resetMatches();
        }

        //        if(termCalculate)
        //            patch.resetMatches();

    } else {
        std::cout << "shares " << patch->matches_->size() <<  " matches @ " <<  patch->x_/16 << " " << patch->y_/16 << std::endl;

        //copy matches and recalculate colorScale!
        patch->copyMatches();
    }


}



cv::Mat SeedMap::debugEpitomeMap() {
    cv::Mat image = debugImages["epitomemap"] = cv::Mat::zeros(baseImage_.size(), CV_8UC3);

    float color = 0;
    float step = 255.0f/charts_.size();
    //*
    foreach(Chart* epi, charts_) {
        foreach(Tile* square, epi->reconTiles_) {
                cv::rectangle(image, square->hull_.verts[0], square->hull_.verts[2],cv::Scalar((128-(int)color) % 255,(255-(int)color) % 255,(int)color,255),-2);
        }
        color += step;
    }
    //*/
    foreach(Tile* square, tiles_) {
        if(square->inUse_) {
            Vector2f v = square->hull_.verts[0];
            int x = v.m_v[0];
            int y = v.m_v[1];
            copyBlock(baseImage_, debugImages["epitomemap"], cv::Rect(x, y, 4, 4), cv::Rect(x,y, 4, 4) );
        }
    }



    return debugImages["epitomemap"];
}

cv::Mat SeedMap::debugReconstruction() {
    debugImages["reconstuct"] = cv::Mat::zeros(sourceImage_.size(), CV_8UC3);

    for(uint i=0; i< blocks_.size(); i++) {
        Patch* patch = blocks_[i];
        int w = patch->s_;
        int h = patch->s_;

        if (!patch->matches_ || patch->matches_->empty()) continue;


        Match* m = patch->matches_->front(); //finalMatch_;
        if(m) {
            cv::Mat reconstruction(m->reconstruct());
            copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, w, h), cv::Rect(patch->x_, patch->y_, w, h) );
        }



    }

    return debugImages["reconstuct"];
}

void SeedMap::setImage(cv::Mat &image) {

    // add patches
    int width =  image.cols / patchS_;
    int height = image.rows / patchS_;
    for(int y=0; y<height; y++)
        for(int x=0; x<width; x++) {
        Patch* block = new Patch( image, sourceGray_, x*patchS_, y*patchS_, patchS_,  1.0f, 0, true);
        blocks_.push_back(block);
    }
}

void SeedMap::addSeedsFromImage(cv::Mat& source, int depth) {

    // add seeds
    float scale = 1.0f;

    for (int z=0; z< depth; z++) {

        float scaleWidth  = source.cols / scale;
        float scaleHeight = source.rows / scale;

        int width =  ((scaleWidth-patchS_) / grid_) + 1;
        int height = ((scaleHeight-patchS_) / grid_) + 1;

        // generate new seeds
        Patch* seed = 0;
        for(int y=0; y<height; y++) {
            int localY = y*grid_;
            for(int x=0; x<width; x++) {
                int localX = x*grid_;
                for(int flip=0; flip<3; flip++) {

                    if (searchInOriginal_ && (localX % patchS_)==0 && (localY % patchS_)==0 && flip==0 && z==0 ) {
                        int indexX = localX / patchS_;
                        int indexY = localY / patchS_;
                        seed = getPatch(indexX, indexY);
                    }
                    else
                        seed = new Patch( source, sourceGray_, localX, localY, patchS_,  scale, flip, false);

                    seeds_.push_back(seed);
                }
            }
        }


        scale *= 1.5f;
    }
    std::cout << blocks_.size() << " " << seeds_.size() << std::endl;
}

void SeedMap::addSeedsFromEpitome() {
    foreach(Chart* epi, charts_) {
        cv::Mat map = epi->getMap();
        addSeedsFromImage(map,1);
    }
}

void SeedMap::setReconSource(cv::Mat& image, int depth) {

    // remove all old seeds
    foreach(Patch* seed, seeds_) {
        delete seed;
    }
    seeds_.clear();

    // load epitomes?
    // add seeds from epitomes

    addSeedsFromImage(image, depth);

}

SeedMap::SeedMap(cv::Mat& image, cv::Mat& base, int s, bool searchInOriginal)
    : termCalculate_(0), patchS_(s), grid_(s/4), matchStep_(0), maxError_(0.0f), searchInOriginal_(searchInOriginal)
{

    // add border to image
    int rightBorder =  patchS_ - (image.cols % patchS_);
    int bottomBorder = patchS_ - (image.rows % patchS_);
    sourceImage_ = cv::Mat::zeros(image.size()+cv::Size(rightBorder,bottomBorder), image.type());
    cv::Mat region(sourceImage_, cv::Rect(cv::Point(0,0),image.size()));
    image.copyTo(region);


    rightBorder =  patchS_ - (base.cols % patchS_);
    bottomBorder = patchS_ - (base.rows % patchS_);
    baseImage_ = cv::Mat::zeros(base.size()+cv::Size(rightBorder,bottomBorder), base.type());
    cv::Mat baseRegion(baseImage_, cv::Rect(cv::Point(0,0),base.size()));
    base.copyTo(baseRegion);


    // create gray version
    cv::cvtColor(sourceImage_, sourceGray_, CV_BGR2GRAY);
    setImage(sourceImage_);
    setReconSource(baseImage_, 3);

}
