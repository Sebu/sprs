#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "glwidget.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    IplImage *image;
    GLWidget *imageWidget;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void changeImage();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
