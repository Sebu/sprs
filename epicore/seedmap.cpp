
#include <opencv/highgui.h>
#include <fstream>
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
            foreach (Patch* innerBlock, match->coveredBlocks_) {
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
        if(block->inChart_ || block->satisfied_ || block->loadsMatches_) continue;

        Chart *chart = new Chart(&baseImage_);

        // mark the whole match region
        foreach(Match *match, block->overlapingMatches_) {
            if(match->block_->inChart_ || match->block_->satisfied_) continue;
            match->block_->satisfied_=true;
            chart->satBlocks_.push_back(match->block_);
            chart->benefit_++;

            foreach(Patch *b, match->coveredBlocks_) {
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
                foreach (Patch* innerBlock, match->coveredBlocks_) {
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

void SeedMap::generateCharts() {

    if(termCalculate_) return;

    genNeighbours();

    uint width = baseImage_.cols/s_;
    uint height = baseImage_.rows/s_;
    foreach(Patch* block, blocks_) {
        if(!block->matches_ || block->finalMatch_) continue;


        foreach(Match* match, *(block->matches_)) {
            // calc bbox of match
            AABB box = match->hull_.getBox();
            uint minx = std::max( (int)(box.min.m_v[0] / s_), 0 );
            uint miny = std::max( (int)(box.min.m_v[1] / s_), 0 );
            uint maxx = std::min( (int)(box.max.m_v[0] / s_), (int)width-1);
            uint maxy = std::min( (int)(box.max.m_v[1] / s_), (int)height-1);


            for(uint y=miny; y<=maxy; y++) {
                for(uint x=minx; x<=maxx; x++) {
                    Patch* b = blocks_.at(y*width + x);

                    if (b->hull_.intersects(match->hull_)) {
                        match->coveredBlocks_.push_back(b);
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

        std::cout << bestChart->benefit_ << std::endl;


        foreach(Patch *b, bestChart->satBlocks_) {
            b->satisfied_=true;
        }

        foreach(Patch *b, bestChart->chartBlocks_) {
            b->inChart_=true;
            b->chart_=bestChart;
        }

        growChart(bestChart);

        charts_.push_back(bestChart);

    };

    optimizeCharts();


}



void SeedMap::optimizeCharts() {

    // find best matches in charts
    foreach(Patch* block, blocks_) {
        if(!block->matches_) continue;
        if(block->loadsMatches_) block->copyMatches();
        std::sort(block->matches_->begin(), block->matches_->end(), matchSorter);
        foreach(Match* match,*block->matches_) {
            bool covered = true;
            Chart *chart = 0;
            foreach(Patch* b, match->coveredBlocks_) {
                if(!b->inChart_) { covered = false; break; }
                if(!chart) chart=b->chart_;
                if(chart!=b->chart_) { covered = false; break; }
            }
            if(covered) {
                block->finalMatch_ = new Match(*match);
                break;
            }
        }
        if(block->loadsMatches_) block->resetMatches();
    }

    // trimm
    //*
    foreach(Patch* block, blocks_) {
        block->inChart_ = false;
    }

    foreach(Patch* block, blocks_) {
        if(block->finalMatch_) {
            foreach(Patch* b, block->finalMatch_->coveredBlocks_)
                b->inChart_ = true;
        }
    }
    //*/
    foreach(Patch *b, blocks_) {
        if(!b->finalMatch_) std::cout<<" missing " << b->x_ << " " << b->y_ << std::endl;
    }

}

void SeedMap::genNeighbours() {
    // neighbours
    // FIXME: :D
    uint width = baseImage_.cols/s_;
    uint height = baseImage_.rows/s_;

    for(uint y=0; y<height; y++) {
        for(uint x=0; x<width; x++) {
            Patch* block = blocks_[y*width+x];

            if (x>0) {
                Patch* n = blocks_[y*width +(x-1)];
                block->neighbours_.push_back(n);
            }
            if (y>0) {
                Patch* n = blocks_[(y-1)*width +x];
                block->neighbours_.push_back(n);
            }
            if(x<width-1) {
                Patch* n = blocks_[y*width +(x+1)];
                block->neighbours_.push_back(n);
            }
            if(y<height-1) {
                Patch* n = blocks_[(y+1)*width +x];
                block->neighbours_.push_back(n);
            }

        }
    }

}


void SeedMap::saveEpitome(std::string fileName) {
    cv::imwrite((fileName + ".epi.png").c_str(), debugEpitomeMap());

    std::ofstream ofs( (fileName + ".epi.txt").c_str() );
    foreach(Chart* c, charts_)
        foreach(Patch* b,c->chartBlocks_)
            ofs << b->x_ << " " << b->y_ << " ";
    ofs.close();
}

// filename
//
void SeedMap::saveCompressedImage(std::string fileName) {
    std::ofstream ofs( (fileName + ".map").c_str() );

    for(uint i=0; i<blocks_.size(); i++)
        blocks_[i]->save(ofs);
    ofs.close();
}


void SeedMap::deserialize(std::string fileName) {
    if (!searchInOriginal_) return;

    std::ifstream ifs( (fileName + ".cache").c_str() );

    if (ifs) {
        float error, s;
        ifs >> s >> grid_ >> error;
        if (crit_.maxError_ == error && s_ == s ) {
            int size;
            ifs >> size;
            for(int i=0; i<size; i++)
                blocks_[i]->deserialize(ifs);

            std::cout << "deserialize done" << std::endl;
        }
    }

    ifs.close();
}


void SeedMap::serialize(std::string fileName) {
    std::ofstream ofs( (fileName + ".cache").c_str() );

    ofs << s_ << " " << grid_ << " " << crit_.maxError_ << " ";
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

    QTime timer;

    timer.start();
    while(!termCalculate_ && matchNext()) {}
    matchStep_=0;


    if(verbose_)
        std::cout << "match time: " <<  timer.elapsed() << std::endl;
}

Patch* SeedMap::matchNext() {
    if (matchStep_>=blocks_.size()) return 0;
    Patch* patch = blocks_[matchStep_++];
    match(patch);
    return patch;
}

Patch* SeedMap::getPatch(int x, int y) {
    Patch* patch = blocks_[y * ((sourceImage_.cols/s_)) + x];
    return patch;
}

void SeedMap::saveReconstruction(std::string fileName) {
    cv::imwrite((fileName + ".recon.png").c_str(), debugReconstruction());
}


void SeedMap::match(Patch* block) {

    block->findFeatures();

    if (!block->matches_) {
        block->matches_ = new std::vector<Match*>;


        bool breakIt = false;

        #pragma omp parallel for
        for(ulong i=0; i< seeds_.size(); i++) {
            Patch* seed = seeds_[i];

            if(!termCalculate_ && !breakIt) {
                Match* match = block->match(*seed);

                if (!match) continue;
                if (match->error_ < crit_.maxError_) {

                    match->calcHull();
                    match->calcPos();

                    if(verbose_) {
                        std::cout << seed->x_ << " " << seed->y_ << " " <<
                                "\t\t orient.: " << "\t\t error: " << match->error_;
                        //std::cout << " " << other.scale_;
                        if(block->x_ ==seed->x_ && block->y_==seed->y_ && seed->isBlock_) std::cout << "\tfound myself!";
                        std::cout << std::endl;
                    }

                     #pragma omp critical
                     block->matches_->push_back(match);

                     if (!match->transformed_ && seed->isBlock_ && searchInOriginal_) {

                         if (!seed->matches_) {
                                seed->matches_=block->matches_;
                                seed->loadsMatches_ = true;
                                block->sharesMatches_ = true;
                         }
                     }

                    if (!searchInOriginal_) {
                        block->finalMatch_ = match;
                        breakIt=true;
                    }


                } else if ( match->error_ > 0.0f && (!block->bestMatch_ ||  match->error_ < block->bestMatch_->error_) ){
                    #pragma omp critical
                    {
                        if(block->bestMatch_) delete block->bestMatch_;
                        block->bestMatch_ = match;
                    }
                } else {
                    delete match;
                }
            }

        }

        if (block->matches_->empty() || termCalculate_) {
            block->resetMatches();
        }

    } else {
        if(verbose_)
            std::cout << "shares " << block->matches_->size() <<  " matches @ " <<  block->x_/s_ << " " << block->y_/s_ << std::endl;

    }

    if (block->matches_ && !block->matches_->empty()) {
        satisfiedBlocks_++;
        if(satisfiedBlocks_==blocks_.size())
            done_=true;
    }


}



cv::Mat SeedMap::debugEpitomeMap() {
    cv::Mat image = debugImages["epitomemap"] = cv::Mat::zeros(baseImage_.size(), CV_8UC3);

    float color = 0;
    float step = 255.0f/charts_.size();
    /*
    foreach(Chart* epi, charts_) {
        foreach(Patch* block, epi->chartBlocks_) {
                cv::rectangle(image, block->hull_.verts[0], block->hull_.verts[2],cv::Scalar((128-(int)color) % 255,(255-(int)color) % 255,(int)color,255),-2);
        }
        color += step;
    }
    //*/
    foreach(Patch* block, blocks_) {
        if(block->inChart_) {
            Vector2f v = block->hull_.verts[0];
            int x = v.m_v[0];
            int y = v.m_v[1];
            copyBlock(baseImage_, debugImages["epitomemap"], cv::Rect(x, y, s_, s_), cv::Rect(x,y, s_, s_) );
        }
    }



    return debugImages["epitomemap"];
}

cv::Mat SeedMap::debugReconstruction() {
    debugImages["reconstuct"] = cv::Mat::zeros(sourceImage_.size(), CV_8UC3);

    if(done_) {
        std::cout << "final match reconstruction" << std::endl;

//    }  else {
        for(uint i=0; i< blocks_.size(); i++) {
            Patch* block = blocks_[i];

            if ((!block->matches_ || block->matches_->empty()) && !block->finalMatch_) continue;


            Match* m = 0;
            if(done_) m = block->finalMatch_;
            else m = block->matches_->front();

            if(m) {
                cv::Mat reconstruction(m->t_.reconstruct(debugImages["epitomemap"], s_));
                copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, s_, s_), cv::Rect(block->x_, block->y_, s_, s_) );
            }

        }

    }

    return debugImages["reconstuct"];
}

void SeedMap::setImage(cv::Mat &image) {

    // add border to image
//    int rightBorder =  s_ - (image.cols % s_);
//    int bottomBorder = s_ - (image.rows % s_);
//    sourceImage_ = cv::Mat::zeros(image.size()+cv::Size(rightBorder,bottomBorder), image.type());
//    cv::Mat region(sourceImage_, cv::Rect(cv::Point(0,0),image.size()));
//    image.copyTo(region);
    sourceImage_ = image;
    // create gray version
    cv::cvtColor(sourceImage_, sourceGray_, CV_BGR2GRAY);

    // add patches
    int width =  (sourceImage_.cols / s_);
    int height = (sourceImage_.rows / s_);
    for(int y=0; y<height; y++)
        for(int x=0; x<width; x++) {
        Patch* block = new Patch( sourceImage_, sourceGray_, x*s_, y*s_, s_,  1.0f, 0, true);
        block->crit_ = &crit_;
        block->verbose_ = verbose_;
        blocks_.push_back(block);
    }
}

void SeedMap::addSeedsFromImage(cv::Mat& source, int depth) {

    // add seeds
    float scale = 1.0f;

    for (uint z=0; z< depth; z++) {

        float scaleWidth  = source.cols / scale;
        float scaleHeight = source.rows / scale;

//        int width =  ((scaleWidth-s_) / grid_) + 1;
//        int height = ((scaleHeight-s_) / grid_) + 1;

        int width =  scaleWidth / grid_;
        int height = scaleHeight / grid_;


        // generate new seeds
        for(uint flip=0; flip<3; flip++) {
            for(uint y=0; y<height; y++) {
                int localY = y*grid_;
                for(uint x=0; x<width; x++) {
                    int localX = x*grid_;
                    Patch* seed = 0;

                    if (searchInOriginal_ && (localX % s_)==0 && (localY % s_)==0 && flip==0 && z==0 ) {
                        int indexX = localX / s_;
                        int indexY = localY / s_;
                        seed = getPatch(indexX, indexY);
                    }
                    if(!seed) {
                        seed = new Patch( source, sourceGray_, localX, localY, s_,  scale, flip, false);
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
    foreach(Chart* epi, charts_) {
        cv::Mat map = epi->getMap();
        addSeedsFromImage(map,1);
    }
}

void SeedMap::setReconSource(cv::Mat& base, int depth) {

    // add border to base
    //    int rightBorder =  s_ - (base.cols % s_);
    //    int bottomBorder = s_ - (base.rows % s_);
    //    baseImage_ = cv::Mat::zeros(base.size()+cv::Size(rightBorder,bottomBorder), base.type());
    //    cv::Mat baseRegion(baseImage_, cv::Rect(cv::Point(0,0),base.size()));
    //    base.copyTo(baseRegion);
    baseImage_ = base;

    // remove all old seeds
    foreach(Patch* seed, seeds_) {
        delete seed;
    }
    seeds_.clear();

    addSeedsFromImage(baseImage_, depth);

}

SeedMap::SeedMap(int s, bool searchInOriginal)
    : termCalculate_(0), s_(s), grid_(s/4), matchStep_(0), searchInOriginal_(searchInOriginal),
    satisfiedBlocks_(0), done_(0), verbose_(1)
{
    std::cout <<  s_ << std::endl;

}
