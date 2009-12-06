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
    IplImage* _image;
    SeedMap*  _seedmap;
    GLWidget* _imageWidget;
    GLWidget* _otherWidget;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void changeImage();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
