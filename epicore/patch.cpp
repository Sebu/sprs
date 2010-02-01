#include <fstream>

#include "patch.h"
#include "match.h"
#include "cv_ext.h"


void Patch::resetMatches() {
    if(!matches) return;
    matches->clear();
    delete matches;
    matches = 0;
}

void Patch::copyMatches() {
    if(!sharesMatches) return;
    std::cout << "correcting colors" << std::endl;

    std::vector<Match*>* newVector = new std::vector<Match*>;
    std::vector<Match*>* oldVector = matches;
    for(uint i=0; i<matches->size(); i++) {
        Match* oldMatch = matches->at(i);
        Match* newMatch = new Match(*oldMatch);

        // recalculate colorScale
        newMatch->colorScale=(cv::Scalar::all(1.0f));
        cv::Mat reconstruction( newMatch->reconstruct() );
        cv::Scalar reconstructionMean = cv::mean(reconstruction);
        for(uint i=0; i<4; i++)
            newMatch->colorScale[i] = this->getHistMean()[i] / reconstructionMean[i];

        newVector->push_back(newMatch);
    }

    matches = newVector;
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

float Patch::reconError(Match* m) {

    float alpha = 1.5f; // 0 <= alpha <= 2
    float beta  = 0.0001f;

    // reconstruct
    cv::Mat reconstruction( m->reconstruct() );

    // 4.1 translation, color scale
    // drop when over bright
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        m->colorScale[i] = this->getHistMean()[i] / reconstructionMean[i];

    if(m->colorScale[0]>1.25f || m->colorScale[1]>1.25f || m->colorScale[2]>1.25f) return 100000.0f;

    reconstruction = cv::Mat( m->reconstruct() );

    // sqaure distance
    cv::Mat diff = cv::abs(this->patchImage - reconstruction);

    float dist=0;
    for (int y=0; y<diff.rows; y++) {
        for(int x=0; x<diff.cols; x++) {
            cv::Vec3b v = diff.at<cv::Vec3b>(y,x);
            float r = v[0]/255.0f;
            float g = v[1]/255.0f;
            float b = v[2]/255.0f;

            dist += (r*r)+(g*g)+(b*b);
        }
    }

    return dist / ( pow( variance, alpha) + beta );
}

void Patch::findFeatures() {

    // precalculate variance
    cv::Scalar mean = cv::mean(grayPatch);
    for (int y=0; y<grayPatch.rows; y++) {
        for(int x=0; x<grayPatch.cols; x++) {
            uchar p = grayPatch.at<uchar>(y,x);
            float v = (p-mean[0])/255.0f;
            variance += v*v;

        }
    }
//    variance /= (grayPatch.cols*grayPatch.rows);

    std::cout << "variance" << variance << std::endl;

    // track initial features
    cv::goodFeaturesToTrack(grayPatch, pointsSrc, 4, .01, .01);
    if(pointsSrc.size()<3)
        std::cout << "too bad features" << std::endl;

}
bool Patch::trackFeatures(Match* match) {

    if(pointsSrc.size()<3) return true;


    cv::Mat grayRotated;
    cv::Mat rotated(match->rotate());

    cv::cvtColor(rotated, grayRotated, CV_BGR2GRAY);


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
            srcTri[index].x = pointsSrc[i].x + match->seedX;
            srcTri[index].y = pointsSrc[i].y + match->seedY;
            destTri[index].x = pointsDest[i].x + match->seedX;
            destTri[index].y = pointsDest[i].y + match->seedY;
            index++;
        }
        if (index>2) break;
    }
    if (index<3) return true;

    cv::Mat tmp = cv::getAffineTransform(destTri, srcTri);


    if(x_==match->seedX && y_==match->seedY) {
    } else {
        cv::Mat selection( match->warpMat, cv::Rect(0,0,3,2) );
        tmp.copyTo(selection);
    }

    return true;

}
Match* Patch::match(Patch& other, float error) {


    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist->minDiff(other.orientHist);

    // orientation still to different
//    if(orientHist->diff(other.orientHist,orientation/5) > 50.0) return 0;

    Match* match = new Match(&other);

    // apply initial rotation TODO: float :D
    if ((int)orientation!=0) {
        cv::Point2f center( (w_/2), (h_/2) );

        cv::Mat rotMat = cv::getRotationMatrix2D(center, -orientation, 1.0f);
        cv::Mat selection( match->rotMat, cv::Rect(0,0,3,2) );
        rotMat.copyTo(selection);

        other.transformed=true;
    }

    // 4.1 KLT matching
    if (!trackFeatures(match)) { delete match; return 0; }

    // 4 reconstruction error
    float reconstructionError =  reconError(match) / (w_*h_);

//    std::cout << reconstructionError << std::endl;

    // reconstruction error too high? skip
    if (reconstructionError > error) {
        delete match;
        return 0;
    }

    match->error = reconstructionError;


    // debug out
#ifdef DEBUG
    std::cout << x_/h_ << " " << y_/h_ << " " <<
            "\t\t orient.: " << orientation << "\t\t error: " << reconstructionError;
    std::cout << " " << other.scale;
    if(x_==other.x_ && y_==other.y_ && other.isPatch()) std::cout << "\tfound myself!";
    std::cout << std::endl;
#endif

    return match;
}

Patch::Patch(cv::Mat& sourceImage, int x, int  y, int w, int h):
        histMean(cv::Scalar::all(0.0f)), x_(x), y_(y), w_(w), h_(h), sharesMatches(0), matches(0),
        sourceImage_(sourceImage), transformed(0), variance(0)
{

    hull = Polygon::square(x,y,w,h);

    patchImage = sourceImage_(cv::Rect(x_,y,w_,h_)).clone();
    cv::cvtColor(patchImage, grayPatch, CV_BGR2GRAY);


    orientHist = new OrientHist(grayPatch, 72);
    setHistMean( cv::mean(patchImage) );


}
