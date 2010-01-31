#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>

#include "orientationhistogram.h"
#include "match.h"

class Match;

class Patch
{
private:
    cv::Scalar histMean;
    std::vector<cv::Point2f> pointsSrc;


public:
    int x_, y_, w_, h_;
    int id;

    float scale;
    bool transformed;


    std::vector<Match*> overlapedMatches;
    Polygon hull;

    std::vector<Match*>* matches;

    cv::Mat patchImage;
    cv::Mat sourceImage_;
    cv::Mat grayPatch;
    float variance;
    OrientHist* orientHist;

    Patch(cv::Mat& sourceImage, int x, int y, int w, int h);

    bool isPatch();

    bool overlaps();
    void findFeatures();
    float reconError(Match*);
    bool trackFeatures(Match*);
    Match* match(Patch&, float);

    cv::Scalar getHistMean() { return histMean; }
    void setHistMean(cv::Scalar _hist_mean) { this->histMean = _hist_mean; }

    void deserialize(std::ifstream&);
    void serialize(std::ofstream&);

};

#endif // SEED_H
