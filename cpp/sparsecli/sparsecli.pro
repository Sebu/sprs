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



INCLUDEPATH += /homes/wheel/seb/playground/include

LIBS += -L/homes/wheel/seb/playground/lib

INCLUDEPATH += /homes/wheel/seb/libsparse/include \
    ../ \
    /opt/local/var/macports/software/opencv/2.1.0_0/opt/local/include
LIBS += \
    -L/opt/local/var/macports/software/opencv/2.1.0_0/opt/local/lib \
    -L../libsparse -llibsparse \
    -lcxcore -lcv -lhighgui -lgomp

SOURCES += main.cpp
