#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include "glwidget.h"
#include "seedmap.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString         fileName;
    cv::Mat         image;
    SeedMap*        seedmap;
    AlbumWidget*    imageWidget;
    AlbumWidget*    _otherWidget;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void testPatch(int x, int y);

public slots:
    void saveImage();
    void changeImage();
    void calculate();
    void calculateTimed();
    void step();
    bool singleStep();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
