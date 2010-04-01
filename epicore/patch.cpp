#include <fstream>
#include <QList>

#include <opencv/highgui.h>

#include "patch.h"
#include "match.h"
#include "cv_ext.h"


bool errorSorter (Feature* i, Feature* j) { return (i->error_ < j->error_ ); }

void Patch::resetMatches() {
    if(!matches_) return;
    matches_->clear();
    delete matches_;
    matches_ = 0;
}

void Patch::correctFinalMatch() {
    if(!sharesMatches_ || !finalMatch_) return;
    if(verbose_)
        std::cout << "correcting color of final match" << std::endl;

    finalMatch_->colorScale_=(cv::Scalar::all(1.0f));
    cv::Mat reconstruction( finalMatch_->reconstruct() );
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        finalMatch_->colorScale_[i] = this->getHistMean()[i] / reconstructionMean[i];

}

void Patch::copyMatches() {
    if(!sharesMatches_) return;
    if(verbose_)
        std::cout << "correcting colors of all matches" << std::endl;

    std::vector<Match*>* newVector = new std::vector<Match*>;

    for(uint i=0; i<matches_->size(); i++) {
        Match* oldMatch = matches_->at(i);
        Match* newMatch = new Match(*oldMatch);
        newMatch->block_ = this;

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


void Patch::deserialize(std::ifstream& ifs) {
    ifs >> x_ >> y_;
    int size;
    ifs >> size;
    if (size==-1) return;
    matches_ = new std::vector<Match*>;
    for(int j=0; j<size; j++) {
        Match* match = new Match(0);
        match->sourceImage_ = sourceColor_;
        match->s_ = s_;
        match -> transScaleFlipMat_ = transScaleFlipMat_;
        match->block_ = this;
        match->deserialize(ifs);
        match->calcHull();
        match->calcPos();
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
        ofs << matches_->size() << " ";
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
        m->colorScale_[i] = this->getHistMean()[i] / reconstructionMean[i];

    if(m->colorScale_[0]>crit_->maxColor_ || m->colorScale_[1]>crit_->maxColor_ || m->colorScale_[2]>crit_->maxColor_) {
        return FLT_MAX;
    }

    float dist=0.0f;
    for (int y=0; y<reconstruction.rows; y++) {

        cv::Vec3b* orig = patchColor_.ptr<cv::Vec3b>(y);
        cv::Vec3b* recon = reconstruction.ptr<cv::Vec3b>(y);
        for(int x=0; x<reconstruction.cols; x++) {

            cv::Vec3b vo = orig[x];
            cv::Vec3b vr = recon[x];
            float r = ( ((float)vr[0]*m->colorScale_[0]) - (float)vo[0] )/255.0f;
            float g = ( ((float)vr[1]*m->colorScale_[1]) - (float)vo[1] )/255.0f;
            float b = ( ((float)vr[2]*m->colorScale_[2]) - (float)vo[2] )/255.0f;

            dist += (r*r)+(g*g)+(b*b);
        }

    }

    return dist / ( pow( variance_, crit_->alpha_) + 0.0000000001f );
}

void Patch::findFeatures() {

    // precalculate variance
    variance_ = 0.0;
    cv::Scalar mean = cv::mean(patchGray_);

    for (int y=0; y<patchGray_.rows; y++) {
        for(int x=0; x<patchGray_.cols; x++) {
            uchar p = cv::saturate_cast<uchar>(patchGray_.at<uchar>(y,x));
            float v = ((float)p-(float)mean[0]) / 255.0f;
            variance_ += v*v;

        }
    }
    variance_ =  sqrt( variance_ / ((patchGray_.cols*patchGray_.rows)-1) );

    std::cout << "variance " << variance_ << std::endl;

    // track initial features


    cv::goodFeaturesToTrack(patchGray_, pointsSrc_, 20, .001, 0.1);

    if(pointsSrc_.size()<3)
        if(verbose_)
            std::cout << "not enougth features found @ " << x_/s_ << " " << y_/s_ << std::endl;

}
bool Patch::trackFeatures(Match* match) {

    //    return true;
    if(pointsSrc_.size()<3) return true;


    cv::Mat rotated(match->warp());

    cv::Mat grayRotated;

    cv::cvtColor(rotated, grayRotated, CV_BGR2GRAY);

    //*/
    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;


    cv::calcOpticalFlowPyrLK( patchGray_, grayRotated,
                              pointsSrc_, pointsDest,
                              status, err,
                              cv::Size(crit_->winSize_,crit_->winSize_), 1, cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 30, 0.01));



    std::vector<Feature*> features;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            features.push_back(new Feature(i,err[i]));
        }
    }

    if (features.size()<3) return true;

//    std::sort(features.begin(), features.end(), errorSorter);
//    cv::Point2f srcTri[3], destTri[3];
    std::vector<cv::Point2f> srcTri, destTri;

    for(uint j = 0; j < features.size(); j++) {
        int i = features[j]->idx_;
        cv::Point2f s,d;
        s.x = pointsSrc_[i].x;
        s.y = pointsSrc_[i].y;
        d.x = pointsDest[i].x;
        d.y = pointsDest[i].y;
//        srcTri[j] = s;
//        destTri[j] = d;
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
        //cv::Mat tmp = cv::estimateRigidTransform(cv::Mat(destTri), cv::Mat(srcTri), true);
        // cv::Mat tmp = cv::getAffineTransform(destTri, srcTri);
        cv::Mat selection( match->warpMat_, cv::Rect(0,0,3,2) );
        tmp.copyTo(selection);
        match->transformed_ = true;
    }
    //*/

    return true;

}
Match* Patch::match(Patch& other) {


    double histDiff = cv::compareHist(colorHist_, other.colorHist_,CV_COMP_CHISQR) / (s_*s_);
//    std::cout << histDiff << std::endl;
    if(histDiff > 1.5) return 0;



    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist_->minDiff(other.orientHist_);

    // orientation still to different
    float diff = orientHist_->diff(other.orientHist_,orientation/orientHist_->factor_);
    //    std::cout << diff << std::endl;
//        if(diff > crit_->maxOrient_) return 0;


    Match* match = new Match(&other);
    match->block_ = this;


    // apply initial rotation
    if ((int)orientation!=0) {
        cv::Point2f center( (s_/2), (s_/2) );

        cv::Mat rotMat = cv::getRotationMatrix2D(center, -orientation, 1.0f);
        cv::Mat selection( match->rotMat_, cv::Rect(0,0,3,2) );
        rotMat.copyTo(selection);
        match->transformed_ = true;
    }

    match->calcTransform();

    // 4.1 KLT matching
    //    if (!) { delete match; return 0; }
    trackFeatures(match);

    match->calcTransform();

    // 4 reconstruction error
    float reconstructionError =  reconError(match) / (s_*s_);

    //    std::cout << reconstructionError << std::endl;

    if(x_ == other.x_ && y_  == other.y_ && reconstructionError > crit_->maxError_ && other.isBlock_) {
        if(verbose_)
            std::cout << other.x_/s_ << " " << other.y_/s_ << " " << orientation << " bad buddy: " << reconstructionError << std::endl;
        std::cout << match->warpMat_.at<double>(0,0) << " " << match->warpMat_.at<double>(0,1) << " " << match->warpMat_.at<double>(0,2) << std::endl;
        std::cout << match->warpMat_.at<double>(1,0) << " " << match->warpMat_.at<double>(1,1) << " " << match->warpMat_.at<double>(1,2) << std::endl;
    }

    // reconstruction error too high? skip
    if (reconstructionError > crit_->maxError_) {
        delete match;
        return 0;
    }

    match->error_ = reconstructionError;
    match->calcHull();
    match->calcPos();

    // debug out
    //*
    if(verbose_) {
        std::cout << other.x_ << " " << other.y_ << " " <<
                "\t\t orient.: " << orientation << "\t\t error: " << reconstructionError;
        //std::cout << " " << other.scale_;
        if(x_==other.x_ && y_==other.y_ && other.isBlock_) std::cout << "\tfound myself!";
        std::cout << std::endl;
    }

    return match;
}

Patch::Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int x, int  y, int s, float scale, int flip, bool isBlock):
        histMean_(cv::Scalar::all(0.0f)), x_(x), y_(y), s_(s), sharesMatches_(0), matches_(0), finalMatch_(0),
        sourceColor_(sourceImage), sourceGray_(sourceGray), transformed_(0), satisfied_(0), variance_(0), isBlock_(isBlock)
{

    size_ = s_ * s_;
    hull_ = Polygon::square(x_,y_,s_,s_);


    cv::Mat transMat = cv::Mat::eye(3,3,CV_64FC1);
    cv::Mat scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    transMat.at<double>(0,2)=-x_;
    transMat.at<double>(1,2)=-y_;
    scaleMat.at<double>(0,0)/=scale;
    scaleMat.at<double>(1,1)/=scale;
    // flip :)
    cv::Mat flipMat = cv::Mat::eye(3,3,CV_64FC1);
    switch(flip) {
    case 1:
        flipMat.at<double>(0,0)=-1.0f;
        flipMat.at<double>(0,2)=s_;
        transformed_ = true;
        break;
    case 2:
        flipMat.at<double>(1,1)=-1.0f;
        flipMat.at<double>(1,2)=s_;
        transformed_ = true;
    default:
        break;
    }

    transScaleFlipMat_ =   (flipMat * scaleMat) * transMat;

    cv::Mat selection(transScaleFlipMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceColor_, patchColor_, selection, cv::Size(s_, s_));
    // cache gray patch version
    cv::cvtColor(patchColor_, patchGray_, CV_BGR2GRAY);

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
    // FIXME: use interface for switching between fast and slow version
    orientHist_ = new OrientHistFast(this, 36);
    setHistMean( cv::mean(patchColor_) );

    if(isBlock_) {
        cv::Mat tmpP;
        cv::Mat selection(transScaleFlipMat_, cv::Rect(0,0,3,2));
        cv::warpAffine(sourceColor_, tmpP, selection, cv::Size(s_, s_));
        cv::cvtColor(tmpP, patchGrayBig_, CV_BGR2GRAY);
    } else {
        patchColor_.release();
        patchGray_.release();
    }

}
