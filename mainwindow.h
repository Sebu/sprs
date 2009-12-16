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
    QString         _fileName;
    IplImage*       _gray;
    IplImage*       _image;
    SeedMap*        _seedmap;
    AlbumWidget*    _imageWidget;
    AlbumWidget*    _otherWidget;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void testPatch(int x, int y);

public slots:
    void changeImage();
    void calculate();
    void singleStep();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
