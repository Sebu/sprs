#ifndef SEEDMAP_H
#define SEEDMAP_H

#include <QList>
#include <QHash>
#include <opencv/cv.h>

#include "patch.h"
#include "transformmap.h"

#include "epicore_global.h"

class EPICORESHARED_EXPORT SeedMap
{
public:
    int patchW;
    int patchH;
    int xgrid;
    int ygrid;
    int width;
    int height;

    float maxError;

    std::vector<Patch*> seeds;
    std::vector<Patch*> patches;
    std::vector<Patch*> matches;

    std::vector<Transform*> transforms;

    cv::Mat sourceImage;

    SeedMap( cv::Mat& image, int w, int h, int xgrid, int ygrid );
    void setImage(cv::Mat& image);

    Patch* getSeed(int x, int y);
    Patch* getPatch(int x, int y);
    void match(Patch& patch);

    void saveReconstruction(std::string filename);

    // debug
    std::map<std::string, cv::Mat> debugImages;
    cv::Mat debugReconstruction();
    cv::Mat debugEpitome();
    cv::Mat debugMean();
    cv::Mat debugOrientation(float delta=1.0f);

};

#endif // SEEDMAP_H
