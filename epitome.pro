# -------------------------------------------------
# Project created by QtCreator 2009-11-25T17:37:11
# -------------------------------------------------
QT += opengl 
TARGET = epitome
TEMPLATE = app
LIBS = -lcv \
    -lhighgui 
SOURCES += main.cpp \
    mainwindow.cpp \
    glwidget.cpp \
    patch.cpp \
    seedmap.cpp \
    cv_ext.cpp \
    orientationhistogram.cpp \
    transformmap.cpp
HEADERS += mainwindow.h \
    glwidget.h \
    patch.h \
    seedmap.h \
    cv_ext.h \
    orientationhistogram.h \
    transformmap.h
FORMS += mainwindow.ui
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg
CONFIG += debug
