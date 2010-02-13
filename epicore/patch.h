#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>

#include "orientationhistogram.h"
#include "orientationhistogramfast.h"
#include "match.h"

class Match;

class Patch
{
private:
    cv::Scalar histMean;
    std::vector<cv::Point2f> pointsSrc;


public:
    static int staticCounter_;
    int id_;

    int x_, y_, w_, h_;

    float scale_;
    bool transformed_;
    bool satisfied_;
    bool inEpitome_;
    Polygon hull_;

    bool sharesMatches_;
    std::vector<Match*>* matches_;



    std::vector<Match*> overlapingMatches;
    std::vector<Patch*> overlapingBlocks;


    cv::Mat sourceImage_;
    cv::Mat sourceGray_;


    cv::Mat patchImage;
    cv::MatND hist;

    cv::Mat grayPatch;
    float variance;
    OrientHistFast* orientHist;

    cv::Mat flipMat;

    bool isPatch_;

    Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int x, int  y, int w, int h, float, int);
    bool overlaps(Vector2f& v);
    bool overlaps(Match* m);



    void findFeatures();
    float reconError(Match*);
    bool trackFeatures(Match*);
    Match* match(Patch&, float);

    void resetMatches();
    void copyMatches();

    cv::Scalar getHistMean() { return histMean; }
    void setHistMean(cv::Scalar _hist_mean) { this->histMean = _hist_mean; }

    void deserialize(std::ifstream&);
    void serialize(std::ofstream&);

};

#endif // SEED_H
