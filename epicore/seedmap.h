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
    bool termCalculate;

    int patchW;
    int patchH;
    int xgrid;
    int ygrid;
    int width;
    int height;

    float maxError;

    uint matchStep;

    std::vector<Patch*> seeds;
    std::vector<Patch*> patches;
    std::list<Epitome*> epitomes;


    cv::Mat sourceImage;

    SeedMap( cv::Mat& image, int w, int h, int xgrid, int ygrid );
    void setImage(cv::Mat&, int depth=1);

    Patch* getSeed(int x, int y);
    Patch* getPatch(int x, int y);
    void match(Patch& patch);

    void saveReconstruction(std::string filename);


    // match tracking
    void resetMatches();
    void matchAll();
    Patch* matchNext();
    void saveMatches(std::string fileName);
    void loadMatches(std::string fileName);

    // epitome generation
    void generateEpitomes();


    // debug
    std::map<std::string, cv::Mat> debugImages;
    cv::Mat debugReconstruction();
    cv::Mat debugEpitomeMap();


};

#endif // SEEDMAP_H
