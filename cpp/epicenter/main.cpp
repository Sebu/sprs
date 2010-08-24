#include <QtGui/QApplication>
#include "mainwindow.h"
#include <X11/Xlib.h>

int main(int argc, char *argv[])
{
    // XInitThreads();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
