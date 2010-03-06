#include <fstream>
#include <QList>

#include <opencv/highgui.h>

#include "patch.h"
#include "match.h"
#include "cv_ext.h"


void Patch::resetMatches() {
    if(!matches_) return;
    matches_->clear();
    delete matches_;
    matches_ = 0;
}

void Patch::copyMatches() {
    if(!sharesMatches_) return;
    if(verbose_)
        std::cout << "correcting colors" << std::endl;

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
        match->s_ = s_;
        match->deserialize(ifs);
        match->block_ = this;
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

    float alpha = 0.9f; // 0 <= alpha <= 2
    float beta  = 0.0000000001f;

    // reconstruct
    cv::Mat reconstruction( m->reconstruct() );

    // 4.1 translation, color scale
    // drop when over bright
    cv::Scalar reconstructionMean = cv::mean(reconstruction);
    for(uint i=0; i<4; i++)
        m->colorScale_[i] = this->getHistMean()[i] / reconstructionMean[i];

    if(m->colorScale_[0]>1.25f || m->colorScale_[1]>1.25f || m->colorScale_[2]>1.25f) {
//        std::cout << reconstructionMean[0] << " " << reconstructionMean[1] <<  " "  <<  reconstructionMean[2] << std::endl;
//        std::cout << m->colorScale_[0] << " " << m->colorScale_[1] <<  " "  <<  m->colorScale_[2] << std::endl;
//        std::cout << m->transformMat_.at<double>(0,0) << " " << m->transformMat_.at<double>(0,1) << " " << m->transformMat_.at<double>(0,2) << std::endl;
//        std::cout << m->transformMat_.at<double>(1,0) << " " << m->transformMat_.at<double>(1,1) << " " << m->transformMat_.at<double>(1,2) << std::endl;
        return FLT_MAX;
    }


    float dist=0.0f;
    for (int y=0; y<reconstruction.rows; y++) {

        cv::Vec3b* orig = patchColor_.ptr<cv::Vec3b>(y);
        cv::Vec3b* recon = reconstruction.ptr<cv::Vec3b>(y);
        for(int x=0; x<reconstruction.cols; x++) {

            cv::Vec3b vo = orig[x];
            cv::Vec3b vr = recon[x];
            float r = ((float)vo[0]-(float)vr[0]*m->colorScale_[0])/255.0f;
            float g = ((float)vo[1]-(float)vr[1]*m->colorScale_[1])/255.0f;
            float b = ((float)vo[2]-(float)vr[2]*m->colorScale_[2])/255.0f;

            dist += (r*r)+(g*g)+(b*b);
        }

    }
//    cv::imshow("fenster123", reconstruction);

//    std::cout << dist << " " << variance_ << std::endl;
    return dist / ( pow( variance_, alpha) + beta );
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

    std::vector<cv::Point2f> points;

    cv::goodFeaturesToTrack(patchGray_, points, 8, .005, 0.1);

    foreach(cv::Point2f newP, points) {
        bool indie = true;

        if(indie)
        pointsSrc_.push_back(newP);
    }


    if(pointsSrc_.size()<3)
        if(verbose_)
            std::cout << "not enougth features found @ " << x_/s_ << " " << y_/s_ << std::endl;

}
bool Patch::trackFeatures(Match* match) {

//    return true;
    if(pointsSrc_.size()<3) return true;


    cv::Mat grayRotated;
    cv::Mat rotated(match->warp());

    cv::cvtColor(rotated, grayRotated, CV_BGR2GRAY);


    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;


    cv::calcOpticalFlowPyrLK( patchGray_, grayRotated,
                              pointsSrc_, pointsDest,
                              status, err,
                              cv::Size(4,4), 1, cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 20, 0.1));

    cv::Point2f srcTri[3], destTri[3];

    // TODO: rewrite status is now a vector
    int index = 0;
    for(uint i = 0; i < status.size(); i++) {
        if(status[i]) {
            srcTri[index].x = pointsSrc_[i].x;
            srcTri[index].y = pointsSrc_[i].y;
            destTri[index].x = pointsDest[i].x;
            destTri[index].y = pointsDest[i].y;
            index++;
        }
        if (index>2) break;
    }
    if (index<3) return true;


    bool same = true;
    for(int i=0; i<3; i++) {
        if(srcTri[i].x!=destTri[i].x ||  srcTri[i].y!=destTri[i].y) {
            same = false;
            break;
        }
    }

    if(!same) {
        cv::Mat tmp = cv::getAffineTransform(destTri, srcTri);
        cv::Mat selection( match->warpMat_, cv::Rect(0,0,3,2) );
        tmp.copyTo(selection);
        match->transformed_ = true;
    }


    return true;

}
Match* Patch::match(Patch& other, float maxError) {


    double histDiff = cv::compareHist(colorHist_, other.colorHist_,CV_COMP_CHISQR);
//    std::cout << histDiff << std::endl;
    if(histDiff > 300) return 0;



    // 4.1 rotation, orientation/gradient histogram
    float orientation = orientHist_->minDiff(other.orientHist_);

    // orientation still to different
    float diff = orientHist_->diff(other.orientHist_,orientation/orientHist_->factor_);
//    std::cout << diff << std::endl;
    if(diff > 150.0) return 0;


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
    trackFeatures(match);
    //    if (!) { delete match; return 0; }

    match->calcTransform();

    // 4 reconstruction error
    float reconstructionError =  reconError(match) / (s_*s_);

//    std::cout << reconstructionError << std::endl;

    if(x_ == other.x_ && y_  == other.y_ && reconstructionError > maxError && other.isBlock_) {
        if(verbose_)
            std::cout << other.x_/s_ << " " << other.x_/s_ << " " << orientation << " bad buddy: " << reconstructionError << std::endl;
            std::cout << match->warpMat_.at<double>(0,0) << " " << match->warpMat_.at<double>(0,1) << " " << match->warpMat_.at<double>(0,2) << std::endl;
            std::cout << match->warpMat_.at<double>(1,0) << " " << match->warpMat_.at<double>(1,1) << " " << match->warpMat_.at<double>(1,2) << std::endl;
    }

    // reconstruction error too high? skip
    if (reconstructionError > maxError) {
        delete match;
        return 0;
    }

    match->error_ = reconstructionError;
    match->calcHull();

    // debug out
    //*
    if(verbose_) {
        std::cout << other.x_/s_ << " " << other.y_/s_ << " " <<
                "\t\t orient.: " << orientation << "\t\t error: " << reconstructionError;
        std::cout << " " << other.scale_;
        if(x_==other.x_ && y_==other.y_ && other.isBlock_) std::cout << "\tfound myself!";
        std::cout << std::endl;
    }

    return match;
}

Patch::Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int x, int  y, int s, float scale, int flip, bool isBlock):
        histMean_(cv::Scalar::all(0.0f)), x_(x), y_(y), s_(s), scale_(scale), sharesMatches_(0), matches_(0), finalMatch_(0),
        sourceColor_(sourceImage), sourceGray_(sourceGray), transformed_(0), satisfied_(0), variance_(0), isBlock_(isBlock)
{

    size_ = s_ * s_;
    hull_ = Polygon::square(x_,y_,s_,s_);


    cv::Mat transMat = cv::Mat::eye(3,3,CV_64FC1);
    cv::Mat scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    transMat.at<double>(0,2)=-x_;
    transMat.at<double>(1,2)=-y_;
    scaleMat.at<double>(0,0)/=scale_;
    scaleMat.at<double>(1,1)/=scale_;
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

    transScaleFlipMat_ =  (flipMat * transMat) * scaleMat;

//    std::cout << transScaleFlipMat_.at<double>(0,0) << " " << transScaleFlipMat_.at<double>(0,1) << " " << transScaleFlipMat_.at<double>(0,2) << std::endl;
//    std::cout << transScaleFlipMat_.at<double>(1,0) << " " << transScaleFlipMat_.at<double>(1,1) << " " << transScaleFlipMat_.at<double>(1,2) << std::endl;

    cv::Mat selection(transScaleFlipMat_, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceColor_, patchColor_, selection, cv::Size(s_, s_));

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

    // cache gray patch version
    cv::cvtColor(patchColor_, patchGray_, CV_BGR2GRAY);

    // generate orientation histogram
    // FIXME: use interface for switching between fast and slow version
    orientHist_ = new OrientHistFast(this, 36);


    setHistMean( cv::mean(patchColor_) );

}
