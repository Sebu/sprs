#-------------------------------------------------
#
# Project created by QtCreator 2010-09-21T14:27:36
#
#-------------------------------------------------

QT       -= core
TARGET = libsparse
TEMPLATE = lib
DEFINES += LIBSPARSE_LIBRARY


# linux
unix:INCLUDEPATH += /homes/wheel/seb/playground/include
unix:LIBS += -L/homes/wheel/seb/playground/lib

# Maces
macx:INCLUDEPATH += /Users/sebastian/uni/diplom/vigra-1.7.0-src/include \
                    /opt/local/var/macports/software/opencv/2.1.0_0/opt/local/include
macx:LIBS += -L/opt/local/var/macports/software/opencv/2.1.0_0/opt/local/lib

# universal
LIBS += -lcxcore -lcv -lhighgui -lgomp

SOURCES += libsparse.cpp \
    dictionary.cpp

HEADERS += libsparse.h\
        libsparse_global.h \
    dictionary.h \
    vigra_ext.h \
    regression.hxx
