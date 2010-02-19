#ifndef SEEDMAP_H
#define SEEDMAP_H

#include <QList>
#include <QHash>
#include <opencv/cv.h>
#include <fstream>

#include "patch.h"
#include "match.h"
#include "epitome.h"
#include "epicore_global.h"

class EPICORESHARED_EXPORT SeedMap
{
public:
    bool termCalculate_;

    int patchS_;
    int grid_;

    bool searchInOriginal_;
    float maxError_;

    uint matchStep_;

    std::vector<Tile*> tiles_;
    std::vector<Patch*> seeds_;
    std::vector<Patch*> blocks_;

    std::list<Chart*> epitomes_;


    cv::Mat sourceImage_;
    cv::Mat sourceGray_;
    cv::Mat baseImage_;

    SeedMap( cv::Mat& image, cv::Mat& base, int s, bool);

    void setImage(cv::Mat& image);
    void setReconSource(cv::Mat& image, int depth);
    void addSeedsFromImage(cv::Mat& source, int depth);
    void addSeedsFromEpitomes();


    Patch* getPatch(int x, int y);
    void match(Patch* patch);

    void saveReconstruction(std::string filename);


    // match tracking
    void resetMatches();
    void matchAll();
    Patch* matchNext();
    void save(std::string fileName);
    void serialize(std::string fileName);
    void deserialize(std::string fileName);

    // epitome generation
    void generateEpitomes();

    // debug
    std::map<std::string, cv::Mat> debugImages;
    cv::Mat debugReconstruction();
    cv::Mat debugEpitomeMap();


};

#endif // SEEDMAP_H
