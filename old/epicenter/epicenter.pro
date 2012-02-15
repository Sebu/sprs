# -------------------------------------------------
# Project created by QtCreator 2009-11-25T17:37:11
# -------------------------------------------------
QT += opengl
TARGET = epicenter
TEMPLATE = app
INCLUDEPATH = /homes/wheel/seb/playground/include \
    /opt/local/var/macports/software/opencv/2.1.0_0/opt/local/include\
    ../
LIBS = -L../epicore \
    -lepicore \
    # -L/homes/wheel/seb/playground/lib \
    -L/opt/local/var/macports/software/opencv/2.1.0_0/opt/local/lib \
    -lcxcore \
    -lcv \
    -lhighgui \
    -lgomp
SOURCES += main.cpp \
    mainwindow.cpp \
    glwidget.cpp \
    calculationthread.cpp
HEADERS += mainwindow.h \
    glwidget.h \
    calculationthread.h
FORMS += mainwindow.ui
QMAKE_CXXFLAGS += -fopenmp
# CONFIG += debug

DEPENDPATH += ../epicore
