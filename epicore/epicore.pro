#-------------------------------------------------
#
# Project created by QtCreator 2010-01-12T18:57:11
#
#-------------------------------------------------

QT       -= gui

TARGET = epicore
TEMPLATE = lib

DEFINES += EPICORE_LIBRARY
debug:DEFINES += DEBUG

LIBS += -lcv \
    -lhighgui

SOURCES += epicore.cpp \
            patch.cpp \
          seedmap.cpp \
         cv_ext.cpp \
         orientationhistogram.cpp \
         transformmap.cpp

HEADERS += epicore.h\
          patch.h \
          seedmap.h \
          cv_ext.h \
          orientationhistogram.h \
          transformmap.h \
          epicore_global.h

QMAKE_CXXFLAGS += -fopenmp
