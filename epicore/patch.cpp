#include <fstream>

#include "patch.h"
#include "transformmap.h"
#include "cv_ext.h"


bool Patch::isPatch() {
    return !(x_ % w_) && !(y_ % h_);
}

void Patch::serialize(std::ofstream& ofs) {
    ofs << x_ << " " << y_ << std::endl;
    ofs << matches->size() << std::endl;
    for(uint j=0; j<matches->size(); j++) {
        matches->at(j)->serialize(ofs);
    }
}

float Patch::reconError(Transform* t) {

    //    Patch* other = t->seed;

    cv::Mat diff;

    // reconstruct
    cv::Mat reconstruction( t->reconstruct() );

    //    cv::imshow("reconstruction", reconstruction);

    // 4.1 translation, color scale
    // atm actually only contrast scale
    // drop when over bright
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        t->colorScale[i] = this->getHistMean()[i] / reconstructionMean[i];
    if(t->colorScale[0]>1.25f) return 100.0f;

    // correct color scale
    std::vector<cv::Mat> planes;
    split(reconstruction, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= t->colorScale.val[i];
    }
    merge(planes,reconstruction);

    // sqaure distance
    diff = cv::abs(this->patchImage - reconstruction);
    cv::pow(diff, 2, diff);


    float sum = (float)cv::sum(diff).val[0];

    cv::Mat mean( patchImage.size(), patchImage.type(), this->getHistMean() );
    cv::Mat varianceMap = cv::abs( patchImage - mean );
    cv::pow(varianceMap, 2, varianceMap);
    float variance = (float)cv::sum(varianceMap).val[0];

    float alpha = 1.0f; // 0 <= alpha <= 2
    float beta  = .02f;

    // error equation
    return sum / ( pow( variance , alpha) + beta );
}

void Patch::findFeatures() {

    cv::Mat grayImage;
    cv::cvtColor(patchImage, grayImage, CV_BGR2GRAY);

    cv::goodFeaturesToTrack(grayImage, pointsSrc, count, .005, .01);

}
bool Patch::trackFeatures(Transform* transform) {

    if(pointsSrc.size()<3) {
        std::cout << "bad pmatch" << std::endl;
        return false;
    }

    cv::Mat grayPatch;
    cv::cvtColor(patchImage, grayPatch, CV_BGR2GRAY);

    cv::Mat grayRotated;
    cv::Mat rotated(transform->rotate());
    cv::cvtColor(rotated(cv::Rect(transform->seed->x_, transform->seed->y_, w_, h_)), grayRotated, CV_BGR2GRAY);

    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;

    cv::calcOpticalFlowPyrLK( grayPatch, grayRotated,
                              pointsSrc, pointsDest,
                              status, err,
                              cv::Size(3,3), 1, cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 10, 0.1));
    //                            cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 40, .1 ), 0 );

    cv::Point2f srcTri[3], destTri[3];

    // TODO: rewrite status is now a vector
    int index = 0;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            srcTri[index].x = pointsSrc[i].x + transform->seed->x_;
            srcTri[index].y = pointsSrc[i].y + transform->seed->y_;
            destTri[index].x = pointsDest[i].x + transform->seed->x_;
            destTri[index].y = pointsDest[i].y + transform->seed->y_;
            index++;
        }
        if (index>2) break;
    }
    if (index<2) return true;

    transform->warpMat = cv::getAffineTransform(destTri, srcTri);

    return true;

}
Transform* Patch::match(Patch& other, float error) {

    Transform* transform = new Transform(&other);

    // 4.1 rotation, orientation/gradient histogram
    float orientation = this->orientHist->minDiff(other.orientHist);

    // apply initial rotation
    cv::Point2f center( other.x_+(w_/2), other.y_+(h_/2) );
    transform->rotMat = cv::getRotationMatrix2D(center, orientation, 1.0f);

    // 4.1 KLT matching
    if (!trackFeatures(transform)) { delete transform; return 0; }

    // 4 reconstruction error
    float reconstuctionError =  reconError(transform);

    if (reconstuctionError > error) { delete transform; return 0; }


    // debug out
    std::cout << x_/h_ << " " << y_/h_ << " " <<
            "color scale: " << transform->colorScale[0] << "\t\t orient.: " << orientation << "\t\t error: " << reconstuctionError;
    std::cout << " " << other.scale;
    if(x_==other.x_ && y_==other.y_) std::cout << "\tfound myself!";
    std::cout << std::endl;

    return transform;
}

Patch::Patch(cv::Mat& _sourceImage, int x, int  y, int w, int h):
        histMean(cv::Scalar::all(0.0f)), x_(x), y_(y), w_(w), h_(h), count(3), sourceImage(_sourceImage), matches(0)
{
    patchImage = sourceImage(cv::Rect(x_,y,w_,h_)).clone();
    cv::Mat grayPatch;
    cv::cvtColor(patchImage, grayPatch, CV_BGR2GRAY);
    orientHist = new OrientHist(grayPatch, 36);
    setHistMean( cv::mean(patchImage) );
}
