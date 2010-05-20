
#include <opencv/highgui.h>
#include <fstream>
#include <tr1/memory>
#include <QTime>

#include "seedmap.h"
#include "cv_ext.h"
#include "epitome.h"


bool tileSorter (Patch* i, Patch* j) { return (i->blocks_ > j->blocks_ ); }
bool chartSorter (Chart* i, Chart* j) { return (i->benefit_ > j->benefit_ ); }

Chart* SeedMap::generateChart(Patch* mostCoveredBlock) {

    std::list<Patch*> potentialChartBlocks;
    potentialChartBlocks.push_back(mostCoveredBlock);

    std::cout<< "start new chart" << std::endl;
    Chart* chart = new Chart(&baseImage_);


    // block coverrage
    while(!potentialChartBlocks.empty()) {
        std::vector<Patch*> deltaI;
        std::vector<Patch*> deltaE;

        //        potentialChartBlocks.sort(tileSorter);
        Patch* currentBlock = potentialChartBlocks.front();
        potentialChartBlocks.pop_front();

        if(currentBlock->inChart_) continue;

        currentBlock->inChart_ = true;
        deltaE.push_back(currentBlock);
        currentBlock->satisfied_ = true;
        deltaI.push_back(currentBlock);

        foreach(Match* match, currentBlock->overlapingMatches_) {
            // block of match allready satisfied?
            if(match->block_->satisfied_) continue;
            match->block_->satisfied_ = true;
            deltaI.push_back(match->block_);

            // mark all covered blocks
            foreach(Patch* block, match->coveredBlocks_) {


                // did we catch other blocks?
                foreach(Match* innerMatch, block->overlapingMatches_) {
                    if(innerMatch->block_->satisfied_) continue;
                    innerMatch->block_->satisfied_ = true;
                    foreach (Patch* innerBlock, innerMatch->coveredBlocks_) {
                        if (innerBlock->inChart_ || innerBlock==block) continue;
                        innerMatch->block_->satisfied_ = false;
                        break;
                    }
                    if (innerMatch->block_->satisfied_)
                        deltaI.push_back(innerMatch->block_);
                }

                if(block->inChart_) continue;
                block->inChart_ = true;
                deltaE.push_back(block);

                if(block->satisfied_) continue;
                block->satisfied_ = true;
                deltaI.push_back(block);


            }


        }

        int benefit = deltaI.size() - deltaE.size();
        if(verbose_)
            std::cout << "benefit: " << deltaI.size() << " " << deltaE.size() << " " << currentBlock->x_/12 << " " << currentBlock->y_/12 << std::endl;

        if(benefit>=0) {
            // add neighbours to potentialChartBlocks
            foreach(Patch* block, deltaE) {
                chart->chartBlocks_.push_back(block);
                foreach(Patch* neighbour, block->neighbours_) {
                    if(!neighbour->inChart_ && !neighbour->satisfied_)
                        potentialChartBlocks.push_back(neighbour);
                }

            }


        } else {
            foreach(Patch* s, deltaE) s->inChart_ = false;
            foreach(Patch* b, deltaI) b->satisfied_ = false;
        }
    }

    return chart;
}

void SeedMap::growChart(Chart *chart) {

}


// create chart candidates
void SeedMap::genCandidateCharts() {


    foreach(Patch *block, blocks_) {
        if(block->satisfied_) continue;
        Chart *chart = new Chart(&baseImage_);

        foreach(Match *match, block->overlapingMatches_) {
            if(match->block_->satisfied_) continue;
            match->block_->satisfied_=true;
            chart->satBlocks_.push_back(match->block_);
            chart->benefit_++;

            foreach(Patch *b, match->coveredBlocks_) {
                if(b->inCandidate_) continue;
                b->inCandidate_=true;
                chart->chartBlocks_.push_back(b);
                chart->benefit_--;
            }
        }

        foreach(Patch *b, chart->satBlocks_) {
            b->satisfied_=false;
        }

        foreach(Patch *b, chart->chartBlocks_) {
            b->inCandidate_=false;
        }

        candidateCharts_.push_back(chart);
    }
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


        genCandidateCharts();
        if(candidateCharts_.empty()) break;


        // sort candidate charts
        candidateCharts_.sort(chartSorter);

        // get bestchart
        Chart* bestChart = candidateCharts_.front();
        candidateCharts_.pop_front();

        std::cout << bestChart->benefit_ << "  " << candidateCharts_.size() << std::endl;

        growChart(bestChart);



        foreach (Chart *c, candidateCharts_) {
            if(c==bestChart) continue;
            delete c;
        }

        foreach(Patch *b, bestChart->satBlocks_) {
            b->satisfied_=true;
        }

        foreach(Patch *b, bestChart->chartBlocks_) {
            b->inChart_=true;
            b->chart_=bestChart;
        }

        charts_.push_back(bestChart);

        candidateCharts_.clear();


    };

    optimizeCharts();


}



void SeedMap::optimizeCharts() {

    // find best matches in charts
    foreach(Patch* block, blocks_) {
        if(!block->matches_) continue;
        std::sort(block->matches_->begin(), block->matches_->end(), matchSorter);
        foreach(Match* match,*block->matches_) {
            bool covered = true;
            Chart *chart = 0;
            foreach(Patch* b, match->coveredBlocks_) {
                if(!b->inChart_) { covered = false; break; }
//                if(!chart) chart=b->chart_;
//                if(chart!=b->chart_) { covered = false; break; }
            }
            if(covered) {
                block->finalMatch_ = new Match(*match);
                block->correctFinalMatch();
                break;
            }
        }
    }

    // trimm
    /*
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

            /*
            if (x>0 && y>0) {
                Patch* n = blocks_[(y-1)*width +(x-1)];
                block->neighbours_.push_back(n);
            }

            if (x>0 && y<height-1) {
                Patch* n = blocks_[(y+1)*width +(x-1)];
                block->neighbours_.push_back(n);
            }

            if (x<width-1 && y>0) {
                Patch* n = blocks_[(y-1)*width +(x+1)];
                block->neighbours_.push_back(n);
            }
            if(x<width-1 && y<height-1) {
                Patch* n = blocks_[(y+1)*width +(x+1)];
                block->neighbours_.push_back(n);
            }
            //*/
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

    //    cv::imshow("grayBig", block->patchGrayBig_);
    //    cv::imshow("gray", block->patchGray_);
    //    cv::imshow("color", block->patchColor_);
    block->findFeatures();

    if (!block->matches_) {
        block->matches_ = new std::vector<Match*>;


        bool breakIt = false;

        #pragma omp parallel for
        for(ulong i=0; i< seeds_.size(); i++) {
            Patch* seed = seeds_[i];

            if(!termCalculate_ && !breakIt) {
                Match* match = block->match(*seed);
                if (match) {

                     #pragma omp critical
                     block->matches_->push_back(match);

                        if (!match->transformed_ && seed->isBlock_ && searchInOriginal_) {

                            if (!seed->matches_) {
                                seed->matches_=block->matches_;
                                seed->sharesMatches_ = true;
                         }
                     }

                    if (!searchInOriginal_) {
                        block->finalMatch_ = match;
                        breakIt=true;
                    }
                }
                //                if(block->matches_->size()>=2000) breakIt=true;
            }

        }

        if (block->matches_->empty() || termCalculate_) {
            block->resetMatches();
        }

    } else {
        if(verbose_)
            std::cout << "shares " << block->matches_->size() <<  " matches @ " <<  block->x_/s_ << " " << block->y_/s_ << std::endl;

        //copy matches and recalculate colorScale!
        //        block->copyMatches();
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
    //*
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

    if(done_) std::cout << "final match reconstruction" << std::endl;
    for(uint i=0; i< blocks_.size(); i++) {
        Patch* block = blocks_[i];


        if (!block->matches_ || block->matches_->empty()) continue;


        Match* m = 0;
        if(done_) m = block->finalMatch_;
        else m = block->matches_->front();

        if(m) {
            cv::Mat reconstruction(m->reconstruct());
            copyBlock(reconstruction, debugImages["reconstuct"] , cv::Rect(0, 0, s_, s_), cv::Rect(block->x_, block->y_, s_, s_) );
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

        int width =  ((scaleWidth-s_) / grid_) + 1;
        int height = ((scaleHeight-s_) / grid_) + 1;

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
