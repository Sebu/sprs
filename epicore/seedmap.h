#ifndef SEEDMAP_H
#define SEEDMAP_H

#include <QList>
#include <QHash>

#include <opencv/cv.h>
#include <fstream>

#include "patch.h"
#include "match.h"
#include "epitome.h"
#include "searchcriteria.h"
#include "epicore_global.h"

class EPICORESHARED_EXPORT SeedMap
{
public:
    bool termCalculate_;

    int s_;
    int grid_;

    int satisfiedBlocks_;
    bool done_;

    bool searchInOriginal_;

    SearchCriteria crit_;

    bool verbose_;

    uint matchStep_;

    std::vector<Patch*> seeds_;
    std::vector<Patch*> blocks_;

    std::list<Chart*> charts_;

    std::list<Chart*> candidateCharts_;

    cv::Mat sourceImage_;
    cv::Mat sourceGray_;
    cv::Mat baseImage_;

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
    void saveEpitome(std::string fileName);
    void saveCompressedImage(std::string fileName);
    void serialize(std::string fileName);
    void deserialize(std::string fileName);

    // epitome generation
    void genNeighbours();
    Chart* generateChart(Patch*);
    void generateCharts();
    void genCandidateCharts();
    void growChart(Chart *chart);
    void optimizeCharts();

    // debug
    std::map<std::string, cv::Mat> debugImages;
    cv::Mat debugReconstruction();
    cv::Mat debugEpitomeMap();


};

#endif // SEEDMAP_H
