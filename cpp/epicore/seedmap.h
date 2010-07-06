#ifndef SEEDMAP_H
#define SEEDMAP_H

#include <QList>
#include <QHash>

#include <opencv/cv.h>
#include <fstream>

#include "patch.h"
#include "match.h"
#include "epitome.h"
#include "epiimage.h"
#include "searchcriteria.h"
#include "epicore_global.h"

class EPICORESHARED_EXPORT SeedMap
{
public:
    bool termCalculate_;

    AABB bbox_;
    int s_;
    int gridstep_;

    int blocksx_;
    int blocksy_;

    long blocksDone_;
    bool done_;

    bool searchInOriginal_;

    SearchCriteria crit_;

    bool verbose_;

    uint matchStep_;

    std::vector<Patch*> seeds_;
    std::vector<Patch*> blocks_;

    EpiImage image_;
    cv::Mat sourceImage_;
    cv::Mat sourceGray_;
    cv::Mat baseImage_;

    std::string fileName_;

    SeedMap(int s, bool);

    void setImage(cv::Mat& image);
    void setReconSource(cv::Mat& image, int depth);
    void addSeedsFromImage(cv::Mat& source, int depth);
    void addSeedsFromEpitome();


    Patch* getPatch(int x, int y);
    void match(Patch* patch);

    void saveReconstruction(std::string filename);


    // match tracking
    void resetMatches();
    void matchAll();
    Patch* matchNext();
    void serialize(std::string fileName);
    void deserialize(std::string fileName);

    // epitome generation
    std::vector<Patch*> genCoveredBlocks(Match *match);
    void findNeighbours();
    Chart* generateChart(Patch*);
    void generateCharts();
    Chart* findBestChart();
    void growChart(Chart *chart);
    void findFinalMatches();

    // debug
    std::map<std::string, cv::Mat> debugImages;
    cv::Mat debugReconstruction();


};

#endif // SEEDMAP_H
