#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>

#include "orientationhistogram.h"
#include "orientationhistogramfast.h"
#include "match.h"

class Match;


class Square
{
public:
    bool inUse_;
    bool done_;
    int blocks_;
    Polygon hull_;
    std::vector<Square*> neighbours_;
    std::vector<Match*> overlapingMatches_;

    Square(int x, int y): inUse_(0), done_(0), blocks_(0) {
        hull_.verts.push_back(Vector2f(x,y));
        hull_.verts.push_back(Vector2f(x+4,y));
        hull_.verts.push_back(Vector2f(x+4,y+4));
        hull_.verts.push_back(Vector2f(x,y+4));
    }

};

class Patch
{
private:
    cv::Scalar histMean;
    std::vector<cv::Point2f> pointsSrc;


public:
    static int staticCounter_;
    int id_;

    int x_, y_, w_, h_;

    int size_;

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
