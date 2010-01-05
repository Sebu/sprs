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
    int _patchW;
    int _patchH;
    int _xgrid;
    int _ygrid;
    int _width;
    int _height;

    float _error;

    QList<Patch*> _seeds;
    QList<Patch*> _matches;

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
