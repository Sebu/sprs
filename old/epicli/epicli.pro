#-------------------------------------------------
#
# Project created by QtCreator 2010-01-13T14:11:58
#
#-------------------------------------------------

QT       -= gui

TARGET = epicli
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH += /homes/wheel/seb/playground/include \
    ../ \
    /opt/local/var/macports/software/opencv/2.1.0_0/opt/local/include
LIBS += \
    # -L/homes/wheel/seb/playground/lib \
    -L/opt/local/var/macports/software/opencv/2.1.0_0/opt/local/lib \
    -L../epicore -lepicore \
    # -L/homes/wheel/seb/playground/lib
    -lcxcore -lcv -lhighgui -lgomp

TEMPLATE = app


SOURCES += main.cpp
