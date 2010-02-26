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
    seedmap->generateEpitome();

    cv::Mat epitomeMap(seedmap->debugEpitomeMap());
    debugWidgetL->fromIpl( epitomeMap, "epitome map" );
    debugWidgetL->updateGL();
}

bool CalculationThread::singleStep(int x, int y) {
    if(!seedmap) init();

    QMutex mutex;
    Patch* patch = 0;
    if(x==-1)
        patch = seedmap->matchNext();
    else {
        int xlocal = ((float)x/400.0) * (seedmap->sourceImage_.size().width / blockSize_);
        int ylocal = ((float)y/400.0) * (seedmap->sourceImage_.size().height / blockSize_);

        patch = seedmap->getPatch(xlocal,ylocal);
        seedmap->match(patch);
    }

    if (!patch) return false;

    if (!patch->matches_) return true;


    cv::Mat tmpImage = base_.clone();

    for(uint i=0; i<patch->matches_->size(); i++) {

        Match* match = patch->matches_->at(i);
        Polygon hull = match->hull_;


        // coverage area
        for (uint j=0; j<seedmap->blocks_.size(); j++) {
            Patch *p = seedmap->blocks_[j];
            if ((p->hull_.intersects(hull)))
                cv::rectangle(tmpImage, p->hull_.verts[0], p->hull_.verts[2], cv::Scalar(0,100,100,100));
        }

        // highlight match
        for(int j=0; j<4; j++)
            cv::line(tmpImage, hull.verts[j], hull.verts[(j+1) % 4], cv::Scalar(0,0,255,100));

    }

    cv::Mat reconstruction(seedmap->debugReconstruction());

    debugWidgetR->fromIpl( tmpImage, "preview" );
    debugWidgetL->fromIpl( reconstruction, "reconstruction" );
    debugWidgetL->updateGL();
    debugWidgetR->updateGL();



    return true;
}

void CalculationThread::init() {
    seedmap = new SeedMap( image_, blockSize_, searchInOriginal_);
    seedmap->setReconSource(base_,3);
    seedmap->maxError_ = error_;

    seedmap->deserialize(fileName);
}

void CalculationThread::calculate() {
    if(!seedmap) init();
    run();
}

void CalculationThread::run() {

    seedmap->resetMatches();

    while(singleStep()) {}

    // FANCY DEBUG outputs
    cv::Mat reconstruction(seedmap->debugReconstruction());
    cv::Mat error( image_.cols, image_.rows, CV_8UC1);
    //    error = image - reconstruction;

    //    debugWidgetL->fromIpl( error, "error");

    debugWidgetL->update();

}
