#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include "glwidget.h"
#include <epicore/seedmap.h>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    std::string     fileName;
    cv::Mat         image;
    SeedMap*        seedmap;
    AlbumWidget*    debugWidgetL;
    AlbumWidget*    debugWidgetR;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void testPatch(int x, int y);

public slots:
    void saveImage();
    void changeImage();
    void calculate();
    void step();
    bool singleStep();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
