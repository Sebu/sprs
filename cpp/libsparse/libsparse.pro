#-------------------------------------------------
#
# Project created by QtCreator 2010-09-21T14:27:36
#
#-------------------------------------------------

QT       -= core
TARGET = libsparse
TEMPLATE = lib
DEFINES += LIBSPARSE_LIBRARY

QMAKE_CXXFLAGS += -fopenmp

# linux
unix:INCLUDEPATH += /homes/wheel/seb/playground/include
unix:LIBS += -L/homes/wheel/seb/playground/lib

# Maces
macx:INCLUDEPATH += /Users/sebastian/uni/diplom/vigra-1.7.0-src/include \
                    /Users/sebastian/uni/OpenCV-2.1.0/include
macx:LIBS += -L/Users/sebastian/uni/OpenCV-2.1.0/lib
# universal
LIBS += -lcxcore -lcv -lhighgui -lgomp

SOURCES += libsparse.cpp \
    dictionary.cpp \
    coder.cpp \
    coderlasso.cpp \
    trainer.cpp \
    trainermairal.cpp \
    vigra_ext.cpp \
    samples.cpp \
    coderomp.cpp

HEADERS += libsparse.h\
        libsparse_global.h \
    dictionary.h \
    vigra_ext.h \
    regression.hxx \
    coder.h \
    coderlasso.h \
    trainer.h \
    trainermairal.h \
    samples.h \
    coderomp.h
