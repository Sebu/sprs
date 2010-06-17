
#include <opencv/highgui.h>
#include <fstream>
#include <iomanip>
#include <tr1/memory>
#include <QTime>

#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"


// bool tileSorter (Patch* i, Patch* j) { return (i->blocks_ > j->blocks_ ); }
bool chartSorter (Chart* i, Chart* j) { return (i->benefit_ > j->benefit_ ); }

void SeedMap::growChart(Chart *chart) {

    std::list<Patch*> newBlocks;

    foreach(Patch *block, chart->chartBlocks_) {
        foreach(Patch *n, block->neighbours_) {
            if(n->inChart_ || n->satisfied_) continue;
            newBlocks.push_back(n);
        }
    }

    while(!newBlocks.empty()) {


        std::vector<Patch*> satBlocks;

        Patch *newBlock = newBlocks.front();
        newBlocks.pop_front();

        if(newBlock->inChart_ || newBlock->satisfied_) continue;
        newBlock->inChart_=true;
        int benefit = -1;

        foreach(Match* match, newBlock->overlapingMatches_) {
            if(match->block_->satisfied_) continue;
            match->block_->satisfied_ = true;
            std::vector<Patch*> coveredBlocks = genCoveredBlocks(match);
            foreach (Patch* innerBlock, coveredBlocks) {
                if (innerBlock->inChart_) continue;
                match->block_->satisfied_ = false;
                break;
            }
            if (match->block_->satisfied_) {
                satBlocks.push_back(match->block_);
                benefit++;
            }
        }

        if(benefit>=0) {
            newBlock->chart_=chart;
            newBlock->inChart_=true;
            chart->chartBlocks_.push_back(newBlock);
            foreach(Patch *n, newBlock->neighbours_) {
                if(n->inChart_ || n->satisfied_) continue;
                newBlocks.push_back(n);
            }
        } else {
            newBlock->inChart_=false;
            foreach(Patch *b, satBlocks) {
                b->satisfied_=false;
            }

        }

    }




}


// create chart candidates
Chart *SeedMap::findBestChart() {

    int bestBenefit = -1000;
    Chart *bestChart = 0;

    foreach(Patch *block, blocks_) {
        if(block->inChart_ || block->satisfied_ || block->parent_) continue;

        Chart *chart = new Chart(baseImage_);

        // mark the whole match region
        foreach(Match *match, block->overlapingMatches_) {
            if(match->block_->inChart_ || match->block_->satisfied_) continue;
            match->block_->satisfied_=true;
            chart->satBlocks_.push_back(match->block_);
            chart->benefit_++;

            std::vector<Patch*> coveredBlocks = genCoveredBlocks(match);
            foreach(Patch *b, coveredBlocks) {
                if(b->candidate_) continue;
                b->candidate_=true;
                chart->chartBlocks_.push_back(b);
                chart->benefit_--;
            }
        }

        // did we satisfy other blocks?
        foreach(Patch* b, chart->chartBlocks_) {
            foreach(Match* match, b->overlapingMatches_) {
                if(match->block_->satisfied_) continue;
                match->block_->satisfied_ = true;
                std::vector<Patch*> coveredBlocks = genCoveredBlocks(match);
                foreach (Patch* innerBlock, coveredBlocks) {
                    if (innerBlock->candidate_) continue;
                    match->block_->satisfied_ = false;
                    break;
                }
                if (match->block_->satisfied_) {
                    chart->satBlocks_.push_back(match->block_);
                    chart->benefit_++;
                }
            }

        }


        foreach(Patch *b, chart->satBlocks_) {
            b->satisfied_=false;
        }

        foreach(Patch *b, chart->chartBlocks_) {
            b->candidate_=false;
        }

        if(chart->benefit_>bestBenefit) {
            bestBenefit = chart->benefit_;
            bestChart = chart;
        } else {
            chart->satBlocks_.clear();
            chart->chartBlocks_.clear();
            delete chart;
            chart = 0;
        }

    }
    return bestChart;
}

std::vector<Patch*> SeedMap::genCoveredBlocks(Match *match) {

    std::vector<Patch*>coveredBlocks;
    // calc bbox of match
    AABB box = match->hull_.getBox();
    uint minx = floor( std::max(box.min.m_v[0] / s_, 0.0f) );
    uint miny = floor( std::max(box.min.m_v[1] / s_, 0.0f) );
    uint maxx = ceil( std::min(box.max.m_v[0] / s_, (float)blocksx_-1.0f) );
    uint maxy = ceil( std::min(box.max.m_v[1] / s_, (float)blocksy_-1.0f) );


    for(uint y=miny; y<=maxy; y++) {
        for(uint x=minx; x<=maxx; x++) {
            Patch* b = blocks_.at(y*blocksx_ + x);

            if (b->hull_.intersects(match->hull_)) {
                coveredBlocks.push_back(b);
            }
        }
    }
    return coveredBlocks;
}

void SeedMap::generateCharts() {

    if(termCalculate_) return;

    //TODO: free some memory
    /*
    foreach(Patch *seed, seeds_) {
        if(!seed->isBlock_) {
            delete seed;
            seed=0;
        }
    }
    //*/

    genNeighbours();

    foreach(Patch* block, blocks_) {
        if(!block->matches_ || block->finalMatch_ || block->parent_) continue;


        foreach(Match* match, *(block->matches_)) {
            // calc bbox of match
            AABB box = match->hull_.getBox();
            uint minx = floor( std::max(box.min.m_v[0] / s_, 0.0f) );
            uint miny = floor( std::max(box.min.m_v[1] / s_, 0.0f) );
            uint maxx = ceil( std::min(box.max.m_v[0] / s_, (float)blocksx_-1.0f) );
            uint maxy = ceil( std::min(box.max.m_v[1] / s_, (float)blocksy_-1.0f) );


            for(uint y=miny; y<=maxy; y++) {
                for(uint x=minx; x<=maxx; x++) {
                    Patch* b = blocks_.at(y*blocksx_ + x);

                    if (b->hull_.intersects(match->hull_)) {
                        b->overlapingMatches_.push_back(match);
                    }
                }
            }
        }
    }

    if(verbose_)
        std::cout <<  "find overlaped done"  << std::endl;



    while(true) {

        Chart *bestChart = findBestChart();
        if(!bestChart) break;

        if(verbose_)
            std::cout << bestChart->benefit_ << std::endl;


        foreach(Patch *b, bestChart->satBlocks_) {
            b->satisfied_=true;
        }

        foreach(Patch *b, bestChart->chartBlocks_) {
            b->inChart_=true;
            b->chart_=bestChart;
        }

        growChart(bestChart);
        bestChart->caclBBox();

        image_.charts_.push_back(bestChart);

    };

    optimizeCharts();

    image_.genTexture();

    for(uint i=0; i<blocksy_; i++) {
        for(uint j=0; j<blocksx_; j++) {
            Patch* block = getPatch(j,i);
            Match* final = block->finalMatch_;

            if(!block->satChart_) std::cout << "what no satChart?" << std::endl;

            cv::Mat selection1(block->satChart_->transform_, cv::Rect(0,0,3,2));
            cv::Mat inverted = cv::Mat::eye(3,3,CV_64FC1);
            cv::Mat selection2(inverted, cv::Rect(0,0,3,2));
            invertAffineTransform(selection1, selection2);

            Transform* t= new Transform();
            t->colorScale_ = final->t_.colorScale_;
            t->transformMat_ = final->t_.transformMat_ * inverted;
            image_.transforms_.push_back(t);
        }
    }


}



void SeedMap::optimizeCharts() {

    // find best matches in charts
    foreach(Patch* block, blocks_) {
        if(!block->matches_) continue;
        if(block->parent_) block->copyMatches();
        std::sort(block->matches_->begin(), block->matches_->end(), matchSorter);
        foreach(Match* match,*block->matches_) {
            bool covered = true;
            Chart *chart = 0;
            std::vector<Patch*> coveredBlocks = genCoveredBlocks(match);
            if(coveredBlocks.empty()) continue;
            foreach(Patch* b, coveredBlocks) {
                if(!b->inChart_) { covered = false; break; }
                if(!chart) chart=b->chart_;
                if(chart!=b->chart_) { covered = false; break; }
            }
            if(covered) {
                block->finalMatch_ = new Match(*match);
                block->satChart_ = chart;
                break;
            }
        }
        if(!block->satChart_) std::cout << "BLOCK " << block->x_ << " " << block->y_ << "NOT COVERED1" << std::endl;
        if(!block->finalMatch_) std::cout << "BLOCK " << block->x_ << " " << block->y_ << "NOT COVERED2" << std::endl;
        if(!block->satChart_ && !block->chart_) std::cout << "BLOCK " << block->x_ << " " << block->y_ << "NOT COVERED3" << std::endl;
        if(block->parent_ || !block->sharesMatches_) block->resetMatches();
    }

    // trimm
    //*
    foreach(Patch* block, blocks_) {
        block->inChart_ = false;
    }

    foreach(Patch* block, blocks_) {
        if(block->finalMatch_) {
            std::vector<Patch*> coveredBlocks = genCoveredBlocks(block->finalMatch_);
            foreach(Patch* b,coveredBlocks)
                b->inChart_ = true;
        }
    }
    //*/
    foreach(Patch *b, blocks_) {
        if(!b->finalMatch_) std::cout<<" missing " << b->x_ << " " << b->y_ << std::endl;
    }

}

void SeedMap::genNeighbours() {

    for(uint y=0; y<blocksy_; y++) {
        for(uint x=0; x<blocksx_; x++) {
            Patch* block = blocks_[y*blocksx_+x];

            if (x>0) {
                Patch* n = blocks_[y*blocksx_ +(x-1)];
                block->neighbours_.push_back(n);
            }
            if (y>0) {
                Patch* n = blocks_[(y-1)*blocksx_ +x];
                block->neighbours_.push_back(n);
            }
            if(x<blocksx_-1) {
                Patch* n = blocks_[y*blocksx_ +(x+1)];
                block->neighbours_.push_back(n);
            }
            if(y<blocksy_-1) {
                Patch* n = blocks_[(y+1)*blocksx_ +x];
                block->neighbours_.push_back(n);
            }

        }
    }

}

void SeedMap::serialize(std::string fileName) {
    std::ofstream ofs( (fileName + ".cache").c_str() );

    ofs << s_ << " " << gridstep_ << " " << crit_.maxError_ << " ";
    ofs << blocks_.size() << " ";
    for(uint i=0; i<blocks_.size(); i++)
        blocks_[i]->serialize(ofs);

    ofs.close();
}

void SeedMap::deserialize(std::string fileName) {
    if (!searchInOriginal_) return;

    std::ifstream ifs( (fileName + ".cache").c_str() );

    if (ifs) {
        float error, s;
        ifs >> s >> gridstep_ >> error;
        if (crit_.maxError_ == error && s_ == s ) {
            int size;
            ifs >> size;
            for(int i=0; i<size; i++)
                blocks_[i]->deserialize(ifs,this);

            std::cout << "deserialize done" << std::endl;
        }
        done_ =true;
    }

    ifs.close();


}


void SeedMap::resetMatches() {
    matchStep_=0;
}

void SeedMap::matchAll() {

    resetMatches();

    QTime timer;

    timer.start();
    long blocks = blocks_.size();
    float step = 100.0f / blocks;
    long last = 0;
    long length = 2;
    long blocksCalced = 0;

    std::cout << std::fixed << std::setprecision(2);
    do {
        last = step*blocksDone_;
        float deltaT = (((float)timer.elapsed() /60000.0f) / (float)blocksCalced) * (blocks-blocksDone_);
        std::cout << "\r [";
        for (long i=0; i<last/length; i++) std::cout << "=";
        std::cout << ">";
        for (long i=0; i<100/length-(last/length); i++) std::cout << ".";
        std::cout << "]" << last  << "%  ETA " << deltaT << "min  " << fileName_ << "         " << std::flush;

        blocksCalced++;
    } while(!termCalculate_ && matchNext());

    std::cout << std::endl;

    std::cout << "match time: " <<  (float)timer.elapsed() / 60000.0f << "min" << std::endl;
}

Patch* SeedMap::matchNext() {
    if (matchStep_>=blocks_.size()) return 0;
    Patch* patch = blocks_[matchStep_++];
    match(patch);
    return patch;
}



Patch* SeedMap::getPatch(int x, int y) {
    Patch* patch = blocks_[y * blocksx_ + x];
    return patch;
}

void SeedMap::saveReconstruction(std::string fileName) {
    cv::imwrite((fileName + ".recon.png").c_str(), debugReconstruction());
}


void SeedMap::match(Patch* block) {

    if (!block->matches_) {
        block->matches_ = new std::vector<Match*>;

        bool breakIt = false;

        #pragma omp parallel for
        for(ulong i=0; i< seeds_.size(); i++) {
            Patch* seed = seeds_[i];

            if(!termCalculate_ && !breakIt) {
                Match* match = block->match(*seed);

                if (match) {

                    match->calcHull();
                    if(match->hull_.inside(bbox_)) {
                        #pragma omp critical
                        block->matches_->push_back(match);

                        if (!match->transformed_ && seed->isBlock_ && searchInOriginal_) {

                            if (!seed->matches_) {
                                blocksDone_++;
                                seed->matches_=block->matches_;
                                seed->parent_ = block;
                                block->sharesMatches_ = true;
                            }
                        }

                        if (!searchInOriginal_) {
                            block->finalMatch_ = match;
                            breakIt=true;
                        }

                    }

                }

            }

        }

        if (block->matches_->empty() || termCalculate_) {
            block->resetMatches();
        }
        blocksDone_++;

    } else {
        if(verbose_)
            std::cout << "shares " << block->matches_->size() <<  " matches @ " <<  block->x_/s_ << " " << block->y_/s_ << std::endl;

    }

    if(blocksDone_==blocks_.size()) done_=true;



}



cv::Mat SeedMap::debugReconstruction() {
    debugImages["reconstuct"] = cv::Mat::ones(sourceImage_.size(), CV_8UC3);


    if(!image_.transforms_.empty()) {

        if(verbose_)
            std::cout << "final match reconstruction" << std::endl;

        image_.reconstruct(debugImages["reconstuct"]);

    }  else {

        foreach(Patch* block,  blocks_) {

            if (!block->matches_ || block->matches_->empty()) continue;

            Match *m = block->matches_->front();

            cv::Mat reconstruction(m->reconstruct());
            copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, s_, s_), cv::Rect(block->x_, block->y_, s_, s_) );
        }

    }

    return debugImages["reconstuct"];
}

void SeedMap::setImage(cv::Mat &image) {

    sourceImage_ = image;

    // add patches
    blocksx_ =  sourceImage_.cols / s_;
    blocksy_ = sourceImage_.rows / s_;

    image_.blocksx_ = blocksx_;
    image_.blocksy_ = blocksy_;
    image_.s_ = s_;

    for(int y=0; y<blocksy_; y++)
        for(int x=0; x<blocksx_; x++) {
        Patch* block = new Patch( sourceImage_,  x*s_, y*s_, s_,  1.0f, 0, true);
        block->crit_ = &crit_;
        block->verbose_ = verbose_;
        block->findFeatures();
        blocks_.push_back(block);
    }
}

void SeedMap::addSeedsFromImage(cv::Mat& source, int depth) {

    // add seeds
    float scale = 1.0f;

    for (uint z=0; z< depth; z++) {

        float scaleWidth  = source.cols / scale;
        float scaleHeight = source.rows / scale;

        int width =  ( ( scaleWidth - ((int)scaleWidth % s_) ) / gridstep_  ) - 3;
        int height = ( ( scaleHeight - ((int)scaleHeight % s_) ) / gridstep_ ) - 3;


        // generate new seeds
        for(uint flip=0; flip<1; flip++) {
            for(uint y=0; y<height; y++) {
                int localY = y*gridstep_;
                for(uint x=0; x<width; x++) {
                    int localX = x*gridstep_;
                    Patch* seed = 0;

                    if (searchInOriginal_ && (localX % s_)==0 && (localY % s_)==0 && flip==0 && z==0 ) {
                        int indexX = localX / s_;
                        int indexY = localY / s_;
                        seed = getPatch(indexX, indexY);
                    }
                    if(!seed) {
                        seed = new Patch( source, localX, localY, s_,  scale, flip, false);
                        seed->crit_ = &crit_;
                        seed->verbose_ = verbose_;
                    }

                    seeds_.push_back(seed);

                }
            }
        }


        scale *= 1.5f;
    }
    if(verbose_)
        std::cout << blocks_.size() << " " << seeds_.size() << std::endl;
}

void SeedMap::addSeedsFromEpitome() {
    foreach(Chart* epi, image_.charts_) {
        cv::Mat map = epi->getMap();
        addSeedsFromImage(map,1);
    }
}

void SeedMap::setReconSource(cv::Mat& base, int depth) {

    baseImage_ = base;

    bbox_.min.m_v[0] = -1.0f;
    bbox_.min.m_v[1] = -1.0f;
    bbox_.max.m_v[0] = baseImage_.cols;
    bbox_.max.m_v[1] = baseImage_.rows;
    // remove all old seeds
    foreach(Patch* seed, seeds_) {
        delete seed;
    }
    seeds_.clear();

    addSeedsFromImage(baseImage_, depth);

}

SeedMap::SeedMap(int s, bool searchInOriginal)
    : termCalculate_(0), s_(s), gridstep_(s/4), matchStep_(0), searchInOriginal_(searchInOriginal),
    done_(0), verbose_(1), blocksDone_(0)
{
}
