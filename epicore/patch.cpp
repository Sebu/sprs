#include <fstream>

#include "patch.h"
#include "transformmap.h"
#include "cv_ext.h"


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
    matches = new std::vector<Transform*>;
    for(uint j=0; j<size; j++) {
        Transform* t = new Transform(0);
        t->sourceImage = this->sourceImage_;
        t->deserialize(ifs);
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

cv::Scalar Patch::reconError(Transform* t) {

    float alpha = 1.0f; // 0 <= alpha <= 2
    float beta  = .02f;
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

    cv::Mat mean( patchImage.size(), patchImage.type(), this->getHistMean() );
    cv::Mat varianceMap = cv::abs( patchImage - mean );
    cv::pow(varianceMap, 2, varianceMap);
    cv::Scalar variance = cv::sum(varianceMap);


    // error equation
    cv::Scalar result;
    for (int i=0; i<3; i++)
        result[i] = sum[i] / ( pow( variance[i] , alpha) + beta );

    return result;
}

void Patch::findFeatures() {

    cv::goodFeaturesToTrack(grayPatch, pointsSrc, count, .01, .01);
    if(pointsSrc.size()<3)
        std::cout << "too bad features" << std::endl;

}
bool Patch::trackFeatures(Transform* transform) {

    if(pointsSrc.size()<3) return true;

    cv::Mat grayRotated;
    cv::Mat rotated(transform->rotate());
    cv::cvtColor(rotated(cv::Rect(transform->seedX, transform->seedY, w_, h_)), grayRotated, CV_BGR2GRAY);

    std::vector<cv::Point2f>    pointsDest;
    std::vector<uchar>          status;
    std::vector<float>          err;

    cv::calcOpticalFlowPyrLK( grayPatch, grayRotated,
                              pointsSrc, pointsDest,
                              status, err,
                              cv::Size(3,3), 1, cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 40, 0.1));
    //                            cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 40, .1 ), 0 );

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
        for (uint i=0; i<3; i++) {
//            std::cout << srcTri[i].x << " " << srcTri[i].y << " ";
//            std::cout << destTri[i].x << " " << destTri[i].y << " ";
        }
//        std::cout << std::endl;

    } else {
        transform->warpMat = tmp;

    }

    return true;

}
Transform* Patch::match(Patch& other, float error) {

    Transform* transform = new Transform(&other);

    // 4.1 rotation, orientation/gradient histogram
    float orientation = this->orientHist->minDiff(other.orientHist);

    // apply initial rotation TODO: float :D
    if ((int)orientation!=0) {
        cv::Point2f center( other.x_+(w_/2), other.y_+(h_/2) );
        transform->rotMat = cv::getRotationMatrix2D(center, orientation, 1.0f);
        other.transformed=true;
    }

    // 4.1 KLT matching
    if (!trackFeatures(transform)) { delete transform; return 0; }

    // 4 reconstruction error
    cv::Scalar reconstructionError =  reconError(transform);

    /*
    if (x_==other.x_ && y_==other.y_) {
        // omg O_o two times the same code
        for (int i=0; i<transform->rotMat.rows; i++)
            for(int j=0; j<transform->rotMat.cols; j++)
                std::cout << transform->rotMat.at<double>(i,j) << " ";
        std::cout << " rotation" << std::endl;
        for (int i=0; i<transform->warpMat.rows; i++)
            for(int j=0; j<transform->warpMat.cols; j++)
                std::cout << transform->warpMat.at<double>(i,j) << " ";
        std::cout << "warp" << std::endl;
        std::cout << orientation << " " << reconstructionError[0] << std::endl;
        std::cout << transform->colorScale[0] << " " << transform->colorScale[1] << " " << transform->colorScale[2] << std::endl;
    }
*/
    if (reconstructionError[0] > error || reconstructionError[1] > error || reconstructionError[2] > error) {

        delete transform;
        return 0;
    }


    // debug out
#ifdef DEBUG
    std::cout << x_/h_ << " " << y_/h_ << " " <<
            "color scale: " << transform->colorScale[0] << "\t\t orient.: " << orientation << "\t\t error: " << reconstructionError[0];
    std::cout << " " << other.scale;
    if(x_==other.x_ && y_==other.y_) std::cout << "\tfound myself!";
    std::cout << std::endl;
#endif

    return transform;
}

Patch::Patch(cv::Mat& sourceImage, int x, int  y, int w, int h):
        histMean(cv::Scalar::all(0.0f)), x_(x), y_(y), w_(w), h_(h), count(4), matches(0), sourceImage_(sourceImage),
        transformed(false)
{
    patchImage = sourceImage_(cv::Rect(x_,y,w_,h_)).clone();
    cv::cvtColor(patchImage, grayPatch, CV_BGR2GRAY);
    orientHist = new OrientHist(grayPatch, 36);
    setHistMean( cv::mean(patchImage) );
}
