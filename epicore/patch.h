#ifndef SEED_H
#define SEED_H

#include <opencv/cv.h>
#include <list>

#include "orientationhistogram.h"
#include "orientationhistogramfast.h"
#include "searchcriteria.h"
#include "match.h"

class Match;


class Tile
{
public:
    int x_, y_;

    Polygon hull_;
    std::vector<Tile*> neighbours_;
    std::vector<Match*> overlapingMatches_;
    std::list<Patch*> overlapingBlocks_;

    /*
    Tile(int x, int y): x_(x), y_(y), inChart_(0), done_(0), blocks_(0) {
        hull_.verts.push_back(Vector2f(x,y));
        hull_.verts.push_back(Vector2f(x+4,y));
        hull_.verts.push_back(Vector2f(x+4,y+4));
        hull_.verts.push_back(Vector2f(x,y+4));
    }
    */

};

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
    int size_;
    float variance_;
    //float scale_;

    SearchCriteria* crit_;

    bool verbose_;

    bool isBlock_;
    bool transformed_;
    bool sharesMatches_;


    bool satisfied_;
    bool inChart_;
    int blocks_;

    Polygon hull_;

    std::string epitomeName_;

    Match* finalMatch_;
    std::vector<Match*>* matches_;

    std::vector<Patch*> neighbours_;
    std::vector<Match*> overlapingMatches_;
    std::list<Patch*> overlapingBlocks_;
    std::list<Patch*> ioverlap_;

    cv::Mat sourceColor_;
    cv::Mat sourceGray_;

    cv::Mat patchColor_;
    cv::Mat patchGray_;
    cv::Mat patchGrayBig_;

    cv::MatND colorHist_;
    OrientHistFast* orientHist_;

    cv::Mat transScaleFlipMat_;

    Patch(cv::Mat& sourceImage, cv::Mat& sourceGray, int, int, int, float, int, bool);

    void findFeatures();
    float reconError(Match*);
    bool trackFeatures(Match*);
    Match* match(Patch&);

    void resetMatches();

    void correctFinalMatch();
    void copyMatches();

    cv::Scalar getHistMean() { return histMean_; }
    void setHistMean(cv::Scalar _hist_mean) { this->histMean_ = _hist_mean; }

    void save(std::ofstream&);
    void serialize(std::ofstream&);
    void deserialize(std::ifstream&);

};

#endif // SEED_H
