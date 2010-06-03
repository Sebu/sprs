#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>
#include <list>

#include "orientationhistogram.h"
#include "orientationhistogramfast.h"
#include "searchcriteria.h"
#include "match.h"

class Match;
class Chart;


class Feature
{
public:
    Feature(int idx, float error) : idx_(idx), error_(error) { }
    int idx_;
    float error_;
};


class Patch
{
private:
    cv::Scalar histMean_;


public:
    std::vector<cv::Point2f> pointsSrc_;

    int x_, y_, s_;
    cv::Mat transScaleFlipMat_;
    int size_;
    float variance_;
    bool isBlock_;
    bool transformed_;
    bool loadsMatches_;
    bool sharesMatches_;
    Polygon hull_;


    SearchCriteria* crit_;
    bool verbose_;

    bool satisfied_;
    bool inChart_;
    bool candidate_;

    Chart* chart_;
    Match* bestMatch_;
    Match* finalMatch_;
    std::vector<Match*>* matches_;
    std::vector<Patch*> neighbours_;
    std::vector<Match*> overlapingMatches_;

    cv::Mat sourceColor_;
    cv::Mat sourceGray_;

    cv::Mat patchColor_;
    cv::Mat patchGray_;

    cv::MatND colorHist_;
    OrientHistFast* orientHist_;


    Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int, int, int, float, int, bool);

    void findFeatures();
    float reconError(Match*);
    bool trackFeatures(Match*);
    Match* match(Patch&);

    void resetMatches();

//    void correctFinalMatch();
    void copyMatches();

    cv::Scalar getHistMean() { return histMean_; }
    void setHistMean(cv::Scalar _hist_mean) { this->histMean_ = _hist_mean; }

    void save(std::ofstream&);
    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);

};

#endif // SEED_H
