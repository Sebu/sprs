#-------------------------------------------------
#
# Project created by QtCreator 2010-09-21T14:27:36
#
#-------------------------------------------------

QT       -= core
TARGET = sprscode
TEMPLATE = lib
DEFINES += LIBSPARSE_LIBRARY

QMAKE_CXXFLAGS += -fopenmp

#QMAKE_CXXFLAGS += -pg
#QMAKE_LFLAGS+= -pg

# linux
unix:INCLUDEPATH += /homes/wheel/seb/playground/include
unix:LIBS += -L/homes/wheel/seb/playground/lib

# Maces
macx:INCLUDEPATH += /Users/sebastian/uni/include \
#                    /Users/sebastian/uni/OpenCV-2.1.0/include
                    /opt/local/include
macx:LIBS += -L/opt/local/lib
#-L/Users/sebastian/uni/OpenCV-2.1.0/lib #\
#            -L/opt/local/lib
# universal
#LIBS += -lcxcore -lcv -lhighgui -lgomp
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lgomp

SOURCES += sprscode.cpp \
    dictionary.cpp \
    coder.cpp \
    coderlasso.cpp \
    trainer.cpp \
    trainermairal.cpp \
    vigra_ext.cpp \
    samples.cpp \
    coderomp.cpp \
    huffman.c \
    rle.c \
    lz.c

HEADERS += sprscode.h \
        sprscode_global.h \
    dictionary.h \
    vigra_ext.h \
    regression.hxx \
    coder.h \
    coderlasso.h \
    trainer.h \
    trainermairal.h \
    samples.h \
    coderomp.h \
    huffman.h \
    rle.h \
    lz.h
