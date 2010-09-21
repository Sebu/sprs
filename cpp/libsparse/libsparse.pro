#-------------------------------------------------
#
# Project created by QtCreator 2010-09-21T14:27:36
#
#-------------------------------------------------

QT       -= core

TARGET = libsparse
TEMPLATE = lib

DEFINES += LIBSPARSE_LIBRARY

SOURCES += libsparse.cpp \
    dictionary.cpp

HEADERS += libsparse.h\
        libsparse_global.h \
    dictionary.h

LIBS += \
    -L/opt/local/var/macports/software/opencv/2.1.0_0/opt/local/lib \
    -lcxcore \
    -lcv \
    -lhighgui \
    -lgomp
