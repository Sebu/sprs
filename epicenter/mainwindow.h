#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include "glwidget.h"
#include "calculationthread.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    std::string     fileName;
    AlbumWidget*    debugWidgetL;
    AlbumWidget*    debugWidgetR;
    CalculationThread calcThread;


    MainWindow(QWidget *parent = 0);
    ~MainWindow();

//    void testPatch(int x, int y);

public slots:
    void saveImage();
    void changeImage();
    void changeBase();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
