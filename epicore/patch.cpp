#include <fstream>

#include "patch.h"
#include "match.h"
#include "cv_ext.h"


bool Patch::overlaps() {
    return true;
}

bool Patch::isPatch() {
    return ( !(x_ % w_) && !(y_ % h_) && !(scale>1.0) && !transformed);
}

void Patch::deserialize(std::ifstream& ifs) {
    ifs >> x_ >> y_;
    ifs.ignore(8192, '\n');
    uint size;
    ifs >> size;
    ifs.ignore(8192, '\n');
    if (size==-1) return;
    matches = new std::vector<Match*>;
    for(uint j=0; j<size; j++) {
        Match* t = new Match(0);
        t->sourceImage = this->sourceImage_;
        t->deserialize(ifs);
        t->patch = this;
        matches->push_back(t);

    }
}

void Patch::serialize(std::ofstream& ofs) {
    ofs << x_ << " " << y_ << std::endl;
    if (matches) {
        ofs << matches->size() << std::endl;
        for(uint j=0; j<matches->size(); j++) {
            matches->at(j)->serialize(ofs);
        }
    } else {
        ofs << -1 << std::endl;
    }
}

cv::Scalar Patch::reconError(Match* t) {

    float alpha = 1.0f; // 0 <= alpha <= 2
    float beta  = .002f;
    cv::Mat diff;

    // reconstruct
    cv::Mat reconstruction( t->reconstruct() );

    // 4.1 translation, color scale
    // atm actually only contrast scale
    // drop when over bright
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        t->colorScale[i] = this->getHistMean()[i] / reconstructionMean[i];

    if(t->colorScale[0]>1.25 || t->colorScale[1]>1.25 || t->colorScale[2]>1.25) return cv::Scalar::all(100.0f);

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
    cv::Scalar sum = cv::sum(diff);


    // error equation
    cv::Scalar result;
    for (int i=0; i<3; i++)
        result[i] = sum[i] / ( pow( variance[i] , alpha) + beta );

    return result;
}

void Patch::findFeatures() {

    // precalculate variance
    cv::Mat mean( patchImage.size(), patchImage.type(), this->getHistMean() );
    cv::Mat varianceMap = cv::abs( patchImage - mean );
    cv::pow(varianceMap, 2, varianceMap);
    variance = cv::sum(varianceMap);

    // track initial features
    cv::goodFeaturesToTrack(grayPatch, pointsSrc, 4, .05, .01);
    if(pointsSrc.size()<3)
        std::cout << "too bad features" << std::endl;

}
bool Patch::trackFeatures(Match* transform) {

    if(pointsSrc.size()<3) return true;


    cv::Mat grayRotated;
    cv::Mat rotated(transform->rotate());

    cv::cvtColor(rotated(cv::Rect(transform->seedX, transform->seedY, transform->seedW, transform->seedH)), grayRotated, CV_BGR2GRAY);


    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;


    cv::calcOpticalFlowPyrLK( grayPatch, grayRotated,
                              pointsSrc, pointsDest,
                              status, err,
                              cv::Size(3,3), 1, cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 40, 0.1));

    cv::Point2f srcTri[3], destTri[3];

    // TODO: rewrite status is now a vector
    int index = 0;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            srcTri[index].x = pointsSrc[i].x + transform->seedX;
            srcTri[index].y = pointsSrc[i].y + transform->seedY;
            destTri[index].x = pointsDest[i].x + transform->seedX;
            destTri[index].y = pointsDest[i].y + transform->seedY;
            index++;
        }
        if (index>2) break;
    }
    if (index<3) return true;

    cv::Mat tmp = cv::getAffineTransform(destTri, srcTri);

    if(x_==transform->seedX && y_==transform->seedY) {
    } else {
        cv::Mat selection( transform->warpMat, cv::Rect(0,0,3,2) );
        tmp.copyTo(selection);
    }

    return true;

}
Match* Patch::match(Patch& other, float error) {


    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist->minDiff(other.orientHist);

    // orientation still to different
    if(orientHist->diff(other.orientHist,orientation/10) > 600) return 0;

    Match* match = new Match(&other);

    // apply initial rotation TODO: float :D
    if ((int)orientation!=0) {
        cv::Point2f center( other.x_+(w_/2), other.y_+(h_/2) );

        cv::Mat rotMat = cv::getRotationMatrix2D(center, orientation, 1.0f);
        cv::Mat selection( match->rotMat, cv::Rect(0,0,3,2) );
        rotMat.copyTo(selection);

        other.transformed=true;
    }

    // 4.1 KLT matching
    if (!trackFeatures(match)) { delete match; return 0; }

    // 4 reconstruction error
    cv::Scalar reconstructionError =  reconError(match);

    match->error = reconstructionError;

    // reconstruction error too high? skip
    if (reconstructionError[0] > error || reconstructionError[1] > error || reconstructionError[2] > error) {

        delete match;
        return 0;

    }


    // debug out
#ifdef DEBUG
    std::cout << x_/h_ << " " << y_/h_ << " " <<
            "\t\t orient.: " << orientation << "\t\t error: " << reconstructionError[0];
    std::cout << " " << other.scale;
    if(x_==other.x_ && y_==other.y_ && other.isPatch()) std::cout << "\tfound myself!";
    std::cout << std::endl;
#endif

    return match;
}

Patch::Patch(cv::Mat& sourceImage, int x, int  y, int w, int h):
        histMean(cv::Scalar::all(0.0f)), x_(x), y_(y), w_(w), h_(h), matches(0), sourceImage_(sourceImage),
        transformed(false)
{

    hull = Polygon::square(x,y,w,h);

    patchImage = sourceImage_(cv::Rect(x_,y,w_,h_)).clone();
    cv::cvtColor(patchImage, grayPatch, CV_BGR2GRAY);
    orientHist = new OrientHist(grayPatch, 36);
    setHistMean( cv::mean(patchImage) );


}
