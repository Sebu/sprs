#ifndef GLWIFGET_H
#define GLWIFGET_H

#include <opencv/cv.h>
#include <QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT        // must include this if you use Qt signals/slots

public:
    GLWidget(QWidget *parent)
            : QGLWidget(parent) { this->setMinimumSize(640,480); }

protected:
    void initializeGL()
    {
        // Set up the rendering context, define display lists etc.:
        IplImage *image = cvCreateImage(cvSize(128,128), IPL_DEPTH_8U, 3);

        unsigned int texImage;
        glGenTextures(1, &texImage);
        glBindTexture(GL_TEXTURE_2D, texImage);

        //GL Texture Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image->width, image->height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, image->imageData);
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    }

    void resizeGL(int w, int h)
    {
        // setup viewport, projection etc.:
        glViewport(0, 0, (GLint)w, (GLint)h);
    }

    void paintGL()
    {
        // draw the scene:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
        glLoadIdentity();

        //Attempts to draw texture to the opened window.
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
        glEnd();
    }
};

#endif // GLWIFGET_H
