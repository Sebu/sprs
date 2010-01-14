#-------------------------------------------------
#
# Project created by QtCreator 2010-01-13T14:11:58
#
#-------------------------------------------------

QT       -= gui

TARGET = epicli
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH = /homes/wheel/seb/playground/include ../
LIBS = -L../epicore -lepicore -L/homes/wheel/seb/playground/lib -lcv \
    -lhighgui

TEMPLATE = app


SOURCES += main.cpp
