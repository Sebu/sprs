#-------------------------------------------------
#
# Project created by QtCreator 2010-09-21T14:42:03
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = sparsecli
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp
#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS+= -pg

# QMAKE_CXXFLAGS += -O3 -msse2 -msse3

# linux
unix:INCLUDEPATH += /homes/wheel/seb/playground/include
unix:LIBS += -L/homes/wheel/seb/playground/lib

# Maces
macx:INCLUDEPATH += /Users/sebastian/uni/include \
                    /Users/sebastian/uni/OpenCV-2.1.0/include
macx:LIBS += -L/Users/sebastian/uni/OpenCV-2.1.0/lib

# universal
INCLUDEPATH += ../
LIBS += -L../sprscode -lsprscode -lgomp
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc

SOURCES += main.cpp

HEADERS +=
