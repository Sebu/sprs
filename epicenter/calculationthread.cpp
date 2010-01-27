#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/patch.h>
#include <epicore/cv_ext.h>

#include "calculationthread.h"

CalculationThread::CalculationThread()
{
}

void CalculationThread::step() {
//    for (int i=0; i<ui->stepSpin->value(); i++)
        singleStep();
}

void CalculationThread::step2() {
    seedmap->generateEpitomes();
    cv::Mat epitomeMap(seedmap->debugEpitomeMap());
    debugWidgetL->fromIpl( epitomeMap, "epitome map" );
    debugWidgetL->updateGL();
}

bool CalculationThread::singleStep(int x, int y) {

    QMutex mutex;
    Patch* patch = 0;
    if(x==-1)
        patch = seedmap->matchNext();
    else {
        int xlocal = ((float)x/400.0) * (seedmap->sourceImage.size().width / blockSize);
        int ylocal = ((float)y/400.0) * (seedmap->sourceImage.size().height / blockSize);

        patch = seedmap->getPatch(xlocal,ylocal);
        seedmap->match(*patch);
    }

    if (!patch) return false;


    // debug
    if (!patch->matches->empty()) {

        cv::Mat tmpImage = seedmap->sourceImage.clone(); // patch->matches->front()->warp();

        // highlight block
        for(int i=0; i<4; i++){
            cv::line(tmpImage, patch->hull.verts[i], patch->hull.verts[(i+1) % 4], cv::Scalar(0,255,0,100),2);
        }

/*
        foreach(Match* match, patch->overlapedMatches) {
            Polygon hull = match->getMatchbox();
            for(int i=0; i<4; i++)
                cv::line(tmpImage, hull.verts[i], hull.verts[(i+1) % 4], cv::Scalar(0,255,255,100));
        }
*/
        for(uint i=0; i<patch->matches->size(); i++) {
            Match* match = patch->matches->at(i);
            Polygon hull = match->getMatchbox();

            for (uint j=0; j<seedmap->patches.size(); j++) {
                Patch *p = seedmap->patches[j];
                if ((p->hull.intersect(hull))) {
//                    match->overlapedPatches.push_back(p);
                    cv::rectangle(tmpImage, p->hull.verts[0], p->hull.verts[2], cv::Scalar(0,100,100,100));
                }

            }

            // highlight match
            for(int j=0; j<4; j++)
                cv::line(tmpImage, hull.verts[j], hull.verts[(j+1) % 4], cv::Scalar(0,0,255,100));

            if(i==0)
                cv::line(tmpImage, hull.verts[0], hull.verts[1], cv::Scalar(255,0,100,100));

        }

        cv::Mat reconstruction(seedmap->debugReconstruction());

        debugWidgetR->fromIpl( tmpImage, "preview" );
        debugWidgetL->fromIpl( reconstruction, "reconstruction" );
        debugWidgetL->updateGL();
        debugWidgetR->updateGL();

    }



    return true;
}

void CalculationThread::calculate() {
    run();
}

void CalculationThread::run() {

    seedmap->resetMatches();

    while(singleStep()) {}

    // FANCY DEBUG outputs
    cv::Mat reconstruction(seedmap->debugReconstruction());
    cv::Mat error( image.cols, image.rows, CV_8UC1);
    //    error = image - reconstruction;

    //    debugWidgetL->fromIpl( error, "error");

    debugWidgetL->update();

}
