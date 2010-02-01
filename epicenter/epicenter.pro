# -------------------------------------------------
# Project created by QtCreator 2009-11-25T17:37:11
# -------------------------------------------------
QT += opengl
TARGET = epicenter
TEMPLATE = app
INCLUDEPATH = /homes/wheel/seb/playground/include \
    ../
LIBS = -L../epicore \
    -lepicore \
    -L/homes/wheel/seb/playground/lib \
    -lcv \
    -lhighgui
SOURCES += main.cpp \
    mainwindow.cpp \
    glwidget.cpp \
    calculationthread.cpp
HEADERS += mainwindow.h \
    glwidget.h \
    calculationthread.h
FORMS += mainwindow.ui
QMAKE_CXXFLAGS += -pg \
    -fopenmp
QMAKE_LFLAGS += -pg
CONFIG += debug

DEPENDPATH += ../epicore
