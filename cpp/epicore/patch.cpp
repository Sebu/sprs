#include <fstream>
#include <QList>

#include <opencv/highgui.h>

#include "patch.h"
#include "match.h"
#include "seedmap.h"
#include "cv_ext.h"

bool errorSorter (Feature* i, Feature* j) { return (i->error_ < j->error_ ); }

int Patch::idCounter_ = 0;

void Patch::resetMatches() {
    if(!matches_) return;
    foreach(Match *m, *matches_)
        delete m;
    matches_->clear();
    delete matches_;
    matches_ = 0;
}


void Patch::copyMatches(cv::Mat& base) {
    if(!parent_) return;
    if(verbose_)
        std::cout << "correcting colors of all matches" << std::endl;

    std::vector<Match*>* newVector = new std::vector<Match*>;

    foreach(Match *oldMatch, *(this->matches_)) {
        Match* newMatch = new Match(*oldMatch);
        newMatch->block_ = this;
        // recalculate error
        Transform t;
        t.colorScale_=cv::Scalar::all(1.0f);
        oldMatch->t_.transformMat_.copyTo(t.transformMat_);
        newMatch->t_ = t;
        newMatch->calcHull();
        cv::Mat reconstruction( newMatch->t_.warp(base, s_) );
        newMatch->error_ =  reconError(newMatch, reconstruction);
        newVector->push_back(newMatch);
    }

    matches_ = newVector;
}

void Patch::serialize(std::ofstream& ofs) {
    ofs << x_ << " " << y_ << " ";

    int mode = 0;
    if (sharesMatches_) mode = PATCH_SHARE;
    if (parent_) mode = PATCH_LOAD;
    ofs << mode << " ";

    if (parent_) {
        int id = parent_->id_;
        ofs << id << " ";
    } else {
        ofs << matches_->size() << " ";
        for(uint j=0; j<matches_->size(); j++) {
            matches_->at(j)->serialize(ofs);
        }
    }
}

void Patch::deserialize(std::ifstream& ifs, SeedMap* map) {
    int mode, size;

    ifs >> x_ >> y_ >> mode >> size;

    switch(mode) {
    case PATCH_LOAD: {
            int id = size;
            parent_ = map->blocks_[id];
            matches_ = parent_->matches_;
            break;
        }
    case PATCH_SHARE:
        sharesMatches_=true;
    default: {
            matches_ = new std::vector<Match*>;
            for(int j=0; j<size; j++) {
                Match* match = new Match(0);
                match->block_ = this;
                match->deserialize(ifs);
                matches_->push_back(match);

            }
            break;
        }
    }


}

void Patch::save(std::ofstream& ofs) {
    if (finalMatch_)
        finalMatch_->save(ofs);
}




float Patch::reconError(Match* m, cv::Mat& reconstruction) {

    // 4.1 translation, color scale
    // drop when over bright
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        m->t_.colorScale_[i] = this->getHistMean()[i] / reconstructionMean[i];

    if(m->t_.colorScale_[0]>1.25f || m->t_.colorScale_[1]>1.25f || m->t_.colorScale_[2]>1.25f) {
        return FLT_MAX;
    }

    float dist=0.0f;
    for (int y=0; y<reconstruction.rows; y++) {

        cv::Vec3b* orig = sourceColor_.ptr<cv::Vec3b>(y+y_);
        cv::Vec3b* recon = reconstruction.ptr<cv::Vec3b>(y);
        for(int x=0; x<reconstruction.cols; x++) {

            cv::Vec3b vo = orig[x+x_];
            cv::Vec3b vr = recon[x];
            float r = ( ((float)vr[0]*m->t_.colorScale_[0]) - (float)vo[0] ) / 255.0;
            float g = ( ((float)vr[1]*m->t_.colorScale_[1]) - (float)vo[1] ) / 255.0;
            float b = ( ((float)vr[2]*m->t_.colorScale_[2]) - (float)vo[2] ) / 255.0;

            dist +=  (r*r)+(g*g)+(b*b);
//            if(dist * errorFactor_ > crit_->maxError_) return FLT_MAX;
        }

    }
    return dist * errorFactor_;
}

void Patch::findFeatures() {

    // precalculate variance

    cv::Mat patchColor;
    cv::Mat patchGray;
    cv::Mat selection(transScaleFlipMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceColor_, patchColor , selection, cv::Size(s_, s_));
    // cache gray patch version
    cv::cvtColor(patchColor, patchGray, CV_RGB2GRAY);


    cv::Scalar mean = cv::mean(patchGray);

    float variance = 0.0;
    for (int y=0; y<patchGray.rows; y++) {
        for(int x=0; x<patchGray.cols; x++) {
            uchar p = cv::saturate_cast<uchar>(patchGray.at<uchar>(y,x));
            float v = ((float)p-(float)mean[0]) / 255.0;
            variance += v*v;

        }
    }

    variance =  variance / (float)(patchGray.cols*patchGray.rows);

//    if(verbose_)
//        std::cout << "variance " << variance << " " << mean[0] << std::endl;

    errorFactor_ = 1.0 / ( pow( variance, crit_->alpha_) + 0.0000000001f );
    errorFactor_ /= s_*s_;

    // track initial features
    cv::goodFeaturesToTrack(patchGray, pointsSrc_,  crit_->gfNumFeatures_, crit_->gfQualityLvl_, crit_->gfMinDist_, cv::Mat(), 5, true);

    int offset = (20-s_)/2;
    for(int i=0; i<pointsSrc_.size(); i++) {
        pointsSrc_[i].x += (float)offset;
        pointsSrc_[i].y += (float)offset;
    }

    if(pointsSrc_.size()<3)
        if(verbose_)
            std::cout << "not enougth features found @ " << x_/s_ << " " << y_/s_ << std::endl;

}
bool Patch::trackFeatures(Patch& other, Match* match) {


    if(pointsSrc_.size()<3) return true;

    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;


    int offset = (20-s_)/2;
    Transform t1;
    cv::Mat offsetMat = cv::Mat::eye(3,3,CV_32FC1);
    offsetMat.at<float>(0,2)=offset;
    offsetMat.at<float>(1,2)=offset;

    t1.transformMat_ = offsetMat * match->t_.transformMat_;
    cv::Mat ret = t1.reconstruct(other.sourceColor_ ,s_+offset+offset);
    cv::Mat reconstruction;
    cv::cvtColor(ret, reconstruction, CV_RGB2GRAY);

    cv::calcOpticalFlowPyrLK( patchGray_, reconstruction,
                              pointsSrc_, pointsDest,
                              status, err,
                              cv::Size(crit_->kltWinSize_,crit_->kltWinSize_), crit_->kltMaxLvls_,
                              cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 40, 0.001));



    std::vector<Feature*> features;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            features.push_back(new Feature(i,err[i]));
        }
    }

    if (features.size()<3) return true;


    std::sort(features.begin(), features.end(), errorSorter);

    cv::Mat P(features.size(),3,CV_32FC1);
    cv::Mat Q(features.size(),3,CV_32FC1);

    for(uint j = 0; j < features.size(); j++) {
        int i = features[j]->idx_;
        P.at<float>(j,0) = pointsDest[i].x;
        P.at<float>(j,1) = pointsDest[i].y;
        P.at<float>(j,2) = 1.0;
        Q.at<float>(j,0) = pointsSrc_[i].x;
        Q.at<float>(j,1) = pointsSrc_[i].y;
        Q.at<float>(j,2) = 1.0;
    }


    cv::Mat tmp = ((P.t()*P).inv() * (P.t() * Q)).t();

    cv::Mat diff = tmp - cv::Mat::eye(3,3,CV_32FC1);

    if(cv::countNonZero(diff)) {
        match->transformed_ = true;
        match->t_.transformMat_ = tmp * match->t_.transformMat_;
    }


    return true;
}


Match* Patch::match(Patch& other) {


    double histDiff = cv::compareHist(colorHist_, other.colorHist_,CV_COMP_CHISQR) / (s_*s_);
    //    std::cout << histDiff << std::endl;
//    if(histDiff > crit_->maxColor_) return 0;



    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist_->minDiff(other.orientHist_);


    // orientation still to different
    float diff = orientHist_->diff(other.orientHist_,orientation/orientHist_->factor_) / (s_*s_);
//    std::cout << diff << std::endl;
//    if(diff > crit_->maxOrient_) return 0;


    Match* match = new Match(&other);
    match->block_ = this;


    // apply initial rotation
    if ((int)orientation!=0) {
        cv::Point2f center( (float)(s_/2)+0.5, (float)(s_/2)+0.5);

        cv::Mat rotMat = cv::Mat::eye(3,3,CV_32FC1);
        cv::Mat rMat = cv::getRotationMatrix2D(center, orientation, 1.0f);
        for(int y=0; y<2; y++)
            for(int x=0; x<3; x++)
                rotMat.at<float>(y,x) = (float)rMat.at<double>(y,x);

        match->t_.transformMat_ = rotMat * match->t_.transformMat_;
        match->transformed_ = true;
    }

//    cv::Mat rec = match->t_.warp(other.sourceColor_, s_);
//    match->error_ =  reconError(match, rec);

//     4.1 KLT matching
    if(id_!=other.id_)
        trackFeatures(other, match);


    cv::Mat selection(match->t_.transformMat_, cv::Rect(0,0,3,2));
    cv::Mat inverted;
    invertAffineTransform(selection, inverted);
    if(cv::countNonZero(inverted)==0) { delete match; return 0; }

    // reconstruct
    cv::Mat reconstruction = match->t_.warp(other.sourceColor_, s_);

    // 4 reconstruction error
    match->error_ =  reconError(match, reconstruction);

    if (match->error_ > crit_->maxError_) {delete match; return 0; }


    //DEBUG
    if(verbose_) {
        std::cout << other.x_ << " " << other.y_ << " " <<
                "\t\t orient.: " << orientation << "\t\t error: " << match->error_;
        //std::cout << " " << other.scale_;
        if(x_ ==other.x_ && y_==other.y_ && other.isBlock_) std::cout << "\tfound myself!";
        std::cout << std::endl;
    }

    return match;
}

Patch::Patch(cv::Mat& sourceImage, int x, int  y, int s, float scale, int flip, bool isBlock):
        histMean_(cv::Scalar::all(0.0f)), x_(x), y_(y), s_(s), parent_(0), sharesMatches_(0), benefit_(1), matches_(0), finalMatch_(0), sourceColor_(sourceImage), transformed_(0), satisfied_(0), inChart_(0), candidate_(0), errorFactor_(0), isBlock_(isBlock)
{

    id_ = idCounter_++;

    hull_ = Polygon::Square(x_,y_,s_,s_);


    cv::Mat transMat = cv::Mat::eye(3,3,CV_32FC1);
    cv::Mat scaleMat = cv::Mat::eye(3,3,CV_32FC1);
    cv::Mat flipMat = cv::Mat::eye(3,3,CV_32FC1);

    transMat.at<float>(0,2)=-x_;
    transMat.at<float>(1,2)=-y_;
    scaleMat.at<float>(0,0)/=scale;
    scaleMat.at<float>(1,1)/=scale;
    // flip :)
    switch(flip) {
    case 1:
        flipMat.at<float>(0,0)=-1.0f;
        flipMat.at<float>(0,2)=s_;
        transformed_ = true;
        break;
    case 2:
        flipMat.at<float>(1,1)=-1.0f;
        flipMat.at<float>(1,2)=s_;
        transformed_ = true;
        break;
    default:
        break;
    }

    transScaleFlipMat_ =   transMat * flipMat * scaleMat;

    cv::Mat patchColor;
    cv::Mat selection(transScaleFlipMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceColor_, patchColor , selection, cv::Size(s_, s_));


    // cache color histogram
    int channels[] = {0, 1, 2};
    int sbins = 8;
    int histSize[] = {sbins, sbins, sbins};
    float sranges[] = { 0, 255 };
    const float* ranges[] = { sranges, sranges, sranges };
    cv::calcHist( &patchColor, 1, channels, cv::Mat(), // do not use mask
                  colorHist_, 3, histSize, ranges,
                  true, // the histogram is uniform
                  false );

    // generate orientation histogram
    orientHist_ = new OrientHistFast(this, 36);
    setHistMean( cv::mean(patchColor) );

    if(isBlock_) {
        int offset = (20-s_)/2;
        Transform t1;
        cv::Mat offsetMat = cv::Mat::eye(3,3,CV_32FC1);
        offsetMat.at<float>(0,2)=offset;
        offsetMat.at<float>(1,2)=offset;
        t1.transformMat_ = offsetMat * transScaleFlipMat_;
        cv::Mat patch = t1.warp(sourceColor_, s_+offset+offset);
        cv::cvtColor(patch, patchGray_, CV_RGB2GRAY);
    }
}
