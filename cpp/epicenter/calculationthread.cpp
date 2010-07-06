#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/patch.h>
#include <epicore/cv_ext.h>

#include "calculationthread.h"

CalculationThread::CalculationThread()
    : seedmap(0), error_(0.0f)
{
}

void CalculationThread::step() {
    //    for (int i=0; i<ui->stepSpin->value(); i++)

    singleStep();
}

void CalculationThread::step2() {
    if(!seedmap) init();
    seedmap->generateCharts();
    cv::Mat epitomeMap(seedmap->image_.Texture());
    debugWidgetL->fromIpl( epitomeMap, "epitome map" );
    debugWidgetL->updateGL();
    cv::Mat recon2(seedmap->debugReconstruction());
    debugWidgetL->fromIpl( recon2, "reconstruction" );
    debugWidgetL->updateGL();
}

bool CalculationThread::singleStep(int x, int y) {
    if(!seedmap) init();

    Patch* block = 0;
    if(x==-1)
        block = seedmap->matchNext();
    else {
        int xlocal = ((float)x/400.0) * (seedmap->sourceImage_.size().width / blockSize_);
        int ylocal = ((float)y/400.0) * (seedmap->sourceImage_.size().height / blockSize_);

        block = seedmap->getPatch(xlocal,ylocal);
        seedmap->match(block);
    }

    if (!block) return false;


    if (!block->matches_) return true;
    cv::Mat tmpImage = base_.clone();

    float colorScale = 255.0 / seedmap->crit_.maxError_;
    if(!block->finalMatch_) {
        for(uint i=0; i<block->matches_->size(); i++) {

            Match* match = block->matches_->at(i);
            Polygon hull = match->hull_;


            // highlight match

            for(int j=0; j<4; j++)
                cv::line(tmpImage, hull.verts[j], hull.verts[(j+1) % 4], cv::Scalar(0,255-match->error_*colorScale,match->error_*colorScale,100));

            foreach(cv::Point2f p, block->pointsSrc_)
                cv::line(tmpImage, p+cv::Point2f(block->x_,block->y_), p+cv::Point2f(block->x_,block->y_), cv::Scalar(0,155,00,100));


        }
    }

    if(block->finalMatch_) {
        // highlight match
        cv::Mat selection(block->finalMatch_->t_.transformMat_, cv::Rect(0,0,3,2));
        cv::Mat inverted;
        invertAffineTransform(selection, inverted);
        for (int i=0; i<2; i++)
            for(int j=0; j<3; j++)
                std::cout << inverted.at<float>(i,j) << " ";
        for(int j=0; j<4; j++)
            std::cout << block->finalMatch_->hull_.verts[j].m_v[0] << " " << block->finalMatch_->hull_.verts[j].m_v[1] << std::endl;
        for(int j=0; j<4; j++)
            cv::line(tmpImage, block->finalMatch_->hull_.verts[j], block->finalMatch_->hull_.verts[(j+1) % 4], cv::Scalar(100,255,0,100));
    }


    debugWidgetR->fromIpl( tmpImage, "preview" );
    debugWidgetR->updateGL();





   cv::Mat reconstruction(seedmap->debugReconstruction());
   debugWidgetL->fromIpl( reconstruction, "reconstruction" );
   debugWidgetL->updateGL();


    return true;
}

void CalculationThread::init() {
    seedmap = new SeedMap(blockSize_, searchInOriginal_);
    seedmap->crit_.maxError_ = error_;
    seedmap->setImage(image_);
    seedmap->setReconSource(base_,3);
    seedmap->deserialize(fileName);
}

void CalculationThread::calculate() {
    if(!seedmap) init();
    run();
}

void CalculationThread::run() {

    seedmap->resetMatches();

    while(singleStep()) {}


}
