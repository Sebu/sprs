#include <fstream>

#include "patch.h"
#include "match.h"
#include "cv_ext.h"


int Patch::staticCounter_ = 0;

bool Patch::overlaps(Vector2f& v) {
    if(v.m_v[0]> x_+w_ || v.m_v[0] < x_ || v.m_v[1] > y_+h_ || v.m_v[1] < y_ ) return false;
    return true;
}

bool Patch::overlaps(Match* m) {
    Polygon p= m->hull_;
    return hull_.intersect(p);
}

void Patch::resetMatches() {
    if(!matches_) return;
    matches_->clear();
    delete matches_;
    matches_ = 0;
}

void Patch::copyMatches() {
    if(!sharesMatches_) return;
    std::cout << "correcting colors" << std::endl;

    std::vector<Match*>* newVector = new std::vector<Match*>;

    for(uint i=0; i<matches_->size(); i++) {
        Match* oldMatch = matches_->at(i);
        Match* newMatch = new Match(*oldMatch);

        // recalculate colorScale
        newMatch->colorScale_=(cv::Scalar::all(1.0f));
        cv::Mat reconstruction( newMatch->reconstruct() );
        cv::Scalar reconstructionMean = cv::mean(reconstruction);
        for(uint i=0; i<4; i++)
            newMatch->colorScale_[i] = this->getHistMean()[i] / reconstructionMean[i];

        newVector->push_back(newMatch);
    }

    matches_ = newVector;
}

bool Patch::isPatch() {

    return ( !(x_ % w_) && !(y_ % h_) && !(scale_>1.0f) );
}



void Patch::deserialize(std::ifstream& ifs) {
    ifs >> x_ >> y_;
    uint size;
    ifs >> size;
    if (size==-1) return;
    matches_ = new std::vector<Match*>;
    for(uint j=0; j<size; j++) {
        Match* match = new Match(0);
        match->sourceImage = sourceImage_;
        match->w_ = w_;
        match->h_ = h_;
        match->deserialize(ifs);
        match->patch = this;
        matches_->push_back(match);

    }
}

void Patch::serialize(std::ofstream& ofs) {
    ofs << x_ << " " << y_ << " ";


    if (matches_) {
        ofs << matches_->size() << " ";
        for(uint j=0; j<matches_->size(); j++) {
            matches_->at(j)->serialize(ofs);
        }
    } else {
        ofs << -1 << " ";
    }
}

float Patch::reconError(Match* m) {

    float alpha = .7f; // 0 <= alpha <= 2
    float beta  = 0.0000000001f;

    // reconstruct
    cv::Mat reconstruction( m->reconstruct() );

    // 4.1 translation, color scale
    // drop when over bright
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        m->colorScale_[i] = this->getHistMean()[i] / reconstructionMean[i];

    if(m->colorScale_[0]>1.25f || m->colorScale_[1]>1.25f || m->colorScale_[2]>1.25f) return 100000.0f;

    reconstruction = cv::Mat( m->reconstruct() );


    // sqaure distance
//    cv::Mat diff= cv::abs(this->patchImage - reconstruction);


    float dist=0.0f;
    for (int y=0; y<reconstruction.rows; y++) {

        cv::Vec3b* orig = patchImage.ptr<cv::Vec3b>(y);
        cv::Vec3b* recon = reconstruction.ptr<cv::Vec3b>(y);
        for(int x=0; x<reconstruction.cols; x++) {

            cv::Vec3b vo = orig[x];
            cv::Vec3b vr = recon[x];
            float r = ((float)vo[0]-(float)vr[0])/255.0f;
            float g = ((float)vo[1]-(float)vr[1])/255.0f;
            float b = ((float)vo[2]-(float)vr[2])/255.0f;

            dist += (r*r)+(g*g)+(b*b);
        }
    }

    return dist; // / ( pow( variance, alpha) + beta );
}

void Patch::findFeatures() {

    // precalculate variance
    variance = 0.0f;
    cv::Scalar mean = cv::mean(grayPatch);
    for (int y=0; y<grayPatch.rows; y++) {
        for(int x=0; x<grayPatch.cols; x++) {
            uchar p = cv::saturate_cast<uchar>(grayPatch.at<uchar>(y,x));
            float v = ((float)p-(float)mean[0]) / 255.0f;
            variance += v*v;

        }
    }
//    variance /= (grayPatch.cols*grayPatch.rows);

//    std::cout << "variance" << variance << std::endl;

    // track initial features
    cv::goodFeaturesToTrack(grayPatch, pointsSrc, 4, .01, .01);
    if(pointsSrc.size()<3)
        std::cout << "too bad features" << std::endl;

}
bool Patch::trackFeatures(Match* match) {

    if(pointsSrc.size()<3) return true;


    cv::Mat grayRotated;
    cv::Mat rotated(match->warp());

    cv::cvtColor(rotated, grayRotated, CV_BGR2GRAY);


    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;


    cv::calcOpticalFlowPyrLK( grayPatch, grayRotated,
                              pointsSrc, pointsDest,
                              status, err,
                              cv::Size(5,5), 1, cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 30, 0.1));

    cv::Point2f srcTri[3], destTri[3];

    // TODO: rewrite status is now a vector
    int index = 0;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            srcTri[index].x = pointsSrc[i].x;
            srcTri[index].y = pointsSrc[i].y;
            destTri[index].x = pointsDest[i].x;
            destTri[index].y = pointsDest[i].y;
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
        match->transformed_ = true;
    }

    return true;

}
Match* Patch::match(Patch& other, float maxError) {




    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist->minDiff(other.orientHist);

    // orientation still to different
    if(orientHist->diff(other.orientHist,orientation/orientHist->factor_) > 50.0) {
        return 0;
    }

    Match* match = new Match(&other);
    match->transformed_ = other.transformed_;

    // apply initial rotation TODO: float :D
    if ((int)orientation!=0) {
        cv::Point2f center( (w_/2), (h_/2) );

        cv::Mat rotMat = cv::getRotationMatrix2D(center, -orientation, 1.0f);
        cv::Mat selection( match->rotMat, cv::Rect(0,0,3,2) );
        rotMat.copyTo(selection);
        match->transformed_ = true;
    }

    match->calcTransform();

    // 4.1 KLT matching
    trackFeatures(match);
//    if (!) { delete match; return 0; }

    match->calcTransform();

    // 4 reconstruction error
    float reconstructionError =  reconError(match) / (w_*h_);

    if(x_ == other.x_ && y_  == other.y_ && reconstructionError > maxError && !(other.scale_>1.0f)) {
        std::cout << orientation << " bad buddy: " << reconstructionError << std::endl;
    }

    // reconstruction error too high? skip
    if (reconstructionError > maxError) {
        delete match;
        return 0;
    }

    match->error_ = reconstructionError;
    match->calcHull();

    // debug out
#ifdef DEBUG
    std::cout << x_/h_ << " " << y_/h_ << " " <<
            "\t\t orient.: " << orientation << "\t\t error: " << reconstructionError;
    std::cout << " " << other.scale_;
    if(x_==other.x_ && y_==other.y_ && other.isPatch()) std::cout << "\tfound myself!";
    std::cout << std::endl;
#endif

    return match;
}

Patch::Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int x, int  y, int w, int h, float scale, int flip):
        histMean(cv::Scalar::all(0.0f)), x_(x), y_(y), w_(w), h_(h), scale_(scale), sharesMatches_(0), matches_(0),
        sourceImage_(sourceImage), sourceGray_(sourceGray), transformed_(0), satisfied_(0), variance(0)
{

    id_ = staticCounter_++;

    hull_ = Polygon::square(x_,y_,w_,h_);
    patchImage = sourceImage_(cv::Rect(x_,y_,w_,h_)).clone();
    // flip :)
    flipMat = cv::Mat::eye(3,3,CV_64FC1);
    switch(flip) {
    case 1:
        cv::flip(patchImage, patchImage, 1);
        flipMat.at<double>(0,0)=-1.0f;
        flipMat.at<double>(0,2)=w;
        transformed_ = true;
        break;
    case 2:
        cv::flip(patchImage, patchImage, 0);
        flipMat.at<double>(1,1)=-1.0f;
        flipMat.at<double>(1,2)=h;
        transformed_ = true;
    default:
        break;
    }

    // cache gray patch version
    cv::cvtColor(patchImage, grayPatch, CV_BGR2GRAY);

    // generate orientation histogram
    // FIXME: use interface for switching between fast and slow version
    orientHist = new OrientHistFast(this, 36);


    setHistMean( cv::mean(patchImage) );


}
