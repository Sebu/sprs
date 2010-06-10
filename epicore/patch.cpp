#include <fstream>
#include <QList>

#include <opencv/highgui.h>

#include "patch.h"
#include "match.h"
#include "cv_ext.h"


bool errorSorter (Feature* i, Feature* j) { return (i->error_ < j->error_ ); }

void Patch::resetMatches() {
    if(!matches_) return;
    foreach(Match *m, *matches_)
        delete m;
    matches_->clear();
    delete matches_;
    matches_ = 0;
}


void Patch::copyMatches() {
    if(!loadsMatches_) return;
    if(verbose_)
        std::cout << "correcting colors of all matches" << std::endl;

    std::vector<Match*>* newVector = new std::vector<Match*>;

    foreach(Match *oldMatch, *(this->matches_)) {
        Match* newMatch = new Match(*oldMatch);
        newMatch->block_ = this;

        // recalculate error
        newMatch->t_.colorScale_=cv::Scalar::all(1.0f);
        newMatch->error_ =  reconError(newMatch) / (s_*s_);
//      if (newMatch->error_ < crit_->maxError_)
        newVector->push_back(newMatch);
    }

    matches_ = newVector;
}


void Patch::deserialize(std::ifstream& ifs) {
    ifs >> x_ >> y_;
    int size;
    ifs >> size;
    if (size==-1) return;
    matches_ = new std::vector<Match*>;
    for(int j=0; j<size; j++) {
        Match* match = new Match(0);
        match->sourceImage_ = sourceColor_;
        transScaleFlipMat_.copyTo(match ->t_.transformMat_);
        match->block_ = this;
        match->deserialize(ifs);
        match->calcHull();
        matches_->push_back(match);

    }
}

void Patch::save(std::ofstream& ofs) {
    if (finalMatch_)
        finalMatch_->save(ofs);
}


void Patch::serialize(std::ofstream& ofs) {
    ofs << x_ << " " << y_ << " ";


    if (matches_) {
        ofs << matches_->size() << " " << std::endl;
        for(uint j=0; j<matches_->size(); j++) {
            matches_->at(j)->serialize(ofs);
        }
    } else {
        ofs << -1 << " ";
    }
}

float Patch::reconError(Match* m) {

    // reconstruct
    cv::Mat reconstruction( m->reconstruct() );

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

        cv::Vec3b* orig = patchColor_.ptr<cv::Vec3b>(y);
        cv::Vec3b* recon = reconstruction.ptr<cv::Vec3b>(y);
        for(int x=0; x<reconstruction.cols; x++) {

            cv::Vec3b vo = orig[x];
            cv::Vec3b vr = recon[x];
            float r = ( ((float)vr[0]*m->t_.colorScale_[0]) - (float)vo[0] )/255.0f;
            float g = ( ((float)vr[1]*m->t_.colorScale_[1]) - (float)vo[1] )/255.0f;
            float b = ( ((float)vr[2]*m->t_.colorScale_[2]) - (float)vo[2] )/255.0f;

            dist += (r*r)+(g*g)+(b*b);
//            if(dist * errorFactor_ > crit_->maxError_) return FLT_MAX;
        }

    }

    return dist * errorFactor_;
}

void Patch::findFeatures() {

    // precalculate variance
    float variance = 0.0;
    cv::Scalar mean = cv::mean(patchGray_);


    for (int y=0; y<patchGray_.rows; y++) {
        for(int x=0; x<patchGray_.cols; x++) {
            uchar p = cv::saturate_cast<uchar>(patchGray_.at<uchar>(y,x));
            float v = ((float)p-(float)mean[0]) / 255.0f;
            variance += v*v;

        }
    }

    variance =  sqrt( variance / ((patchGray_.cols*patchGray_.rows)-1) );

    if(verbose_)
        std::cout << "variance " << variance << std::endl;

    errorFactor_ = 1.0 / ( pow( variance, crit_->alpha_) + 0.0000000001f );
    errorFactor_ /= s_*s_;

    // track initial features
    cv::goodFeaturesToTrack(patchGray_, pointsSrc_, 10, 0.02, 1.0, cv::Mat());
//    cv::goodFeaturesToTrack(patchGray_, pointsSrc_,  crit_->gfNumFeatures_, crit_->gfQualityLvl_, crit_->gfMinDist_);

    if(pointsSrc_.size()<3)
        if(verbose_)
            std::cout << "not enougth features found @ " << x_/s_ << " " << y_/s_ << std::endl;

}
bool Patch::trackFeatures(Match* match) {

    //    return true;
    if(pointsSrc_.size()<3) return true;


    cv::Mat rotated(match->warp());

    cv::Mat grayRotated;
    cv::cvtColor(rotated, grayRotated, CV_RGB2GRAY);

    //*/
    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;


    cv::calcOpticalFlowPyrLK( patchGray_, grayRotated,
                              pointsSrc_, pointsDest,
                              status, err,
                              cv::Size(crit_->kltWinSize_,crit_->kltWinSize_), crit_->kltMaxLvls_,
                              cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 50, 0.01));



    std::vector<Feature*> features;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            features.push_back(new Feature(i,err[i]));
        }
    }

    if (features.size()<3) return true;


    std::sort(features.begin(), features.end(), errorSorter);

    std::vector<cv::Point2f> srcTri, destTri;

    for(uint j = 0; j < 3; j++) {
        int i = features[j]->idx_;
        cv::Point2f s,d;
        s.x = pointsSrc_[i].x;
        s.y = pointsSrc_[i].y;
        d.x = pointsDest[i].x;
        d.y = pointsDest[i].y;
        srcTri.push_back(s);
        destTri.push_back(d);
    }



    bool same = true;
    for(int i=0; i<3; i++) {
        if(srcTri[i].x!=destTri[i].x ||  srcTri[i].y!=destTri[i].y) {
            same = false;
            break;
        }
    }



    if(!same) {
        cv::Mat tmp = getTransform(destTri, srcTri);
        // cv::Mat tmp = cv::estimateRigidTransform(cv::Mat(destTri), cv::Mat(srcTri), true);
        // cv::Mat tmp = cv::getAffineTransform(destTri, srcTri);
        cv::Mat warpMat = cv::Mat::eye(3,3,CV_64FC1);
        cv::Mat selection( warpMat, cv::Rect(0,0,3,2) );
        tmp.copyTo(selection);

        match->t_.transformMat_ = warpMat * match->t_.transformMat_;

        match->transformed_ = true;
    }
    //*/

    return true;

}
Match* Patch::match(Patch& other) {


     double histDiff = cv::compareHist(colorHist_, other.colorHist_,CV_COMP_CHISQR) / (s_*s_);
//    std::cout << histDiff << std::endl;
     if(histDiff > crit_->maxColor_) return 0;



    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist_->minDiff(other.orientHist_);


    // orientation still to different
    float diff = orientHist_->diff(other.orientHist_,orientation/orientHist_->factor_);
//    std::cout << diff << std::endl;
    if(diff > crit_->maxOrient_) return 0;


    Match* match = new Match(&other);
    match->block_ = this;


    // apply initial rotation
    if ((int)orientation!=0) {
        cv::Point2f center( (float)(s_/2), (float)(s_/2) );

        cv::Mat rMat = cv::Mat::eye(3,3,CV_64FC1);
        cv::Mat rotMat = cv::getRotationMatrix2D(center, orientation, 1.0f);
        cv::Mat selection( rMat, cv::Rect(0,0,3,2) );
        rotMat.copyTo(selection);
        match->t_.transformMat_ = rMat * match->t_.transformMat_;
        match->transformed_ = true;
    }

    // 4.1 KLT matching
    trackFeatures(match);

    // 4 reconstruction error
    match->error_ =  reconError(match);


    //DEBUG
    if(x_ == other.x_ && y_  == other.y_ && match->error_ > crit_->maxError_ && other.isBlock_) {
        if(verbose_)
            std::cout << other.x_/s_ << " " << other.y_/s_ << " " << orientation << " bad buddy: " << match->error_ << std::endl;
//        std::cout << match->warpMat_.at<double>(0,0) << " " << match->warpMat_.at<double>(0,1) << " " << match->warpMat_.at<double>(0,2) << std::endl;
//        std::cout << match->warpMat_.at<double>(1,0) << " " << match->warpMat_.at<double>(1,1) << " " << match->warpMat_.at<double>(1,2) << std::endl;
    }

    return match;
}

Patch::Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int x, int  y, int s, float scale, int flip, bool isBlock):
        histMean_(cv::Scalar::all(0.0f)), x_(x), y_(y), s_(s), loadsMatches_(0), sharesMatches_(0), matches_(0), bestMatch_(0), finalMatch_(0), sourceColor_(sourceImage), transformed_(0), satisfied_(0), inChart_(0), candidate_(0), chart_(0), satChart_(0), errorFactor_(0), isBlock_(isBlock)
{

    size_ = s_ * s_;
    hull_ = Polygon::Square(x_,y_,s_,s_);


    cv::Mat transMat = cv::Mat::eye(3,3,CV_64FC1);
    cv::Mat scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    cv::Mat flipMat = cv::Mat::eye(3,3,CV_64FC1);

    transMat.at<double>(0,2)=-x_;
    transMat.at<double>(1,2)=-y_;
    scaleMat.at<double>(0,0)/=scale;
    scaleMat.at<double>(1,1)/=scale;
    // flip :)
    switch(flip) {
    case 1:
        flipMat.at<double>(0,0)=-1.0f;
        flipMat.at<double>(0,2)=s_-1;
        transformed_ = true;
        break;
    case 2:
        flipMat.at<double>(1,1)=-1.0f;
        flipMat.at<double>(1,2)=s_-1;
        transformed_ = true;
        break;
    default:
        break;
    }

    transScaleFlipMat_ =   (flipMat * scaleMat) * transMat;

    cv::Mat selection(transScaleFlipMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceColor_, patchColor_, selection, cv::Size(s_, s_));
    // cache gray patch version
    cv::cvtColor(patchColor_, patchGray_, CV_RGB2GRAY);



    // cache color histogram
    int channels[] = {0, 1, 2};
    int sbins = 8;
    int histSize[] = {sbins, sbins, sbins};
    float sranges[] = { 0, 255 };
    const float* ranges[] = { sranges, sranges, sranges };
    cv::calcHist( &patchColor_, 1, channels, cv::Mat(), // do not use mask
                  colorHist_, 3, histSize, ranges,
                  true, // the histogram is uniform
                  false );


    // generate orientation histogram
    orientHist_ = new OrientHistFast(this, 36);
    setHistMean( cv::mean(patchColor_) );

    if(!isBlock_) {
        patchColor_.release();
        patchGray_.release();
    }

}
