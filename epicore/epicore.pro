# -------------------------------------------------
# Project created by QtCreator 2010-01-12T18:57:11
# -------------------------------------------------
QT -= gui
CONFIG += debug
TARGET = epicore
TEMPLATE = lib
DEFINES += EPICORE_LIBRARY
debug:DEFINES += DEBUG
INCLUDEPATH += /homes/wheel/seb/playground/include
LIBS += -L/homes/wheel/seb/playground/lib \
    -lcv \
    -lhighgui
SOURCES += epicore.cpp \
    patch.cpp \
    seedmap.cpp \
    cv_ext.cpp \
    orientationhistogram.cpp \
    match.cpp \
    matchconsumer.cpp \
    matrix.cpp \
    epitome.cpp
HEADERS += epicore.h \
    patch.h \
    seedmap.h \
    cv_ext.h \
    orientationhistogram.h \
    match.h \
    epicore_global.h \
    matchconsumer.h \
    matrix.h \
    epitome.h
QMAKE_CXXFLAGS += -fopenmp
