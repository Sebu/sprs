#ifndef CACULATIONTHREAD_H
#define CACULATIONTHREAD_H

#include <QThread>
#include <epicore/seedmap.h>

#include "glwidget.h"

class CalculationThread : public QThread
{
    Q_OBJECT

public:
    cv::Mat         image_;
    cv::Mat         base_;
    std::string     fileName;
    SeedMap*        seedmap;
    AlbumWidget*    debugWidgetL;
    AlbumWidget*    debugWidgetR;
    int blockSize_;
    bool searchInOriginal_;
    float error_;

    CalculationThread();
    void init();
    void run();

public slots:
    void calculate();
    void step();
    bool singleStep(int x=-1, int y=-1);
    void step2();
};

#endif // CACULATIONTHREAD_H
