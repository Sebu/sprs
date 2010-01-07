#ifndef SEEDMAP_H
#define SEEDMAP_H

#include <QList>
#include <QHash>
#include <opencv/cv.h>

#include "patch.h"
#include "transformmap.h"

class SeedMap
{
public:
    int patchW;
    int patchH;
    int xgrid;
    int ygrid;
    int width;
    int height;

    float maxError;

    QList<Patch*> seeds;
    QList<Patch*> patches;
    QList<Patch*> matches;

    QList<Transform*> transforms;

    AlbumWidget* _debugAlbum;
    AlbumWidget* _debugAlbumR;

    cv::Mat sourceImage;

    QHash<QString, cv::Mat> debugImages;

    SeedMap( cv::Mat& image, int w, int h, int xgrid, int ygrid );
    void setImage(cv::Mat& image);

    Patch* at(int x, int y);

    void testPatch(int x, int y);
    void match(Patch& patch);

    // debug
    cv::Mat reconstructIpl();
    cv::Mat epitomeIpl();
    cv::Mat meanIpl();
    cv::Mat orientIpl(float delta=1.0f);

};

#endif // SEEDMAP_H
