# -------------------------------------------------
# Project created by QtCreator 2010-01-12T18:57:11
# -------------------------------------------------
QT -= gui

TARGET = epicore
TEMPLATE = lib

DEFINES += EPICORE_LIBRARY


INCLUDEPATH += /homes/wheel/seb/playground/include \
    /opt/local/var/macports/software/opencv/2.1.0_0/opt/local/include
LIBS += \
    # -L/homes/wheel/seb/playground/lib \
    -L/opt/local/var/macports/software/opencv/2.1.0_0/opt/local/lib \
    -lcxcore \
    -lcv \
    -lhighgui \
    -lgomp

SOURCES += epicore.cpp \
    patch.cpp \
    seedmap.cpp \
    cv_ext.cpp \
    orientationhistogram.cpp \
    orientationhistogramfast.cpp \
    match.cpp \
    matchconsumer.cpp \
    matrix.cpp \
    epitome.cpp \
    searchcriteria.cpp \
    epiimage.cpp
HEADERS += epicore.h \
    patch.h \
    seedmap.h \
    cv_ext.h \
    orientationhistogram.h \
    orientationhistogramfast.h \
    match.h \
    epicore_global.h \
    matchconsumer.h \
    matrix.h \
    epitome.h \
    searchcriteria.h \
    epiimage.h
