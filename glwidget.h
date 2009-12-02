#ifndef GLWIFGET_H
#define GLWIFGET_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QGLWidget>
#include <QtGui>

class GLWidget : public QGLWidget
{
Q_OBJECT

public:
    unsigned int texImage;


public:
    GLWidget(QWidget *parent)
            : QGLWidget(parent) {
        glGenTextures(1, &texImage);
        setMinimumSize(400,400);
    }

    int fromIpl(IplImage *image) {
        makeCurrent();
        if (image==NULL) return -1;

        glBindTexture( GL_TEXTURE_2D, texImage );

        GLenum input_format;

        switch(image->nChannels) {
            case 1: input_format = GL_LUMINANCE; break;
            case 3: input_format = GL_BGR; break;
            case 4: input_format = GL_BGRA; break;
            default: input_format = GL_BGR; break;
        }

        GLenum channel_depth;

        switch(image->depth) {
            case IPL_DEPTH_8U: channel_depth  = GL_UNSIGNED_BYTE; break;
            case IPL_DEPTH_32F: channel_depth = GL_FLOAT; break;
            default: channel_depth = GL_UNSIGNED_BYTE; break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, input_format, channel_depth, image->imageData);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

        return 0;

    }

public slots:


protected:
    void initializeGL()
    {
        glEnable(GL_TEXTURE_2D);
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
    }

    void resizeGL(int w, int h)
    {
        glViewport(0, 0, (GLint)w, (GLint)h);
    }

    void paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
        glEnd();
    }
};

#endif // GLWIFGET_H
