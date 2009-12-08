#ifndef SEEDMAP_H
#define SEEDMAP_H

#include <QList>
#include "patch.h"
#include <opencv/cv.h>

class SeedMap
{
public:
    int _xgrid;
    int _ygrid;
    int _width;
    int _height;

    QList<Patch*> _seeds;
    QList<Patch*> _matches;

    IplImage* _sourceImage;

    IplImage* _epitome_ipl;
    IplImage* _mean_ipl;
    IplImage* _orient_ipl;

    SeedMap(IplImage* image, int xoffset, int yoffset );
    void setImage(IplImage* image);

    Patch* at(int x, int y);

    void match(Patch& patch);

    // debug
    IplImage* epitomeIpl();
    IplImage* meanIpl();
    IplImage* orientIpl(float delta=1.0f);

};

#endif // SEEDMAP_H
