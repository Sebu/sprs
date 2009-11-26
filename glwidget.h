#ifndef GLWIFGET_H
#define GLWIFGET_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QGLWidget>
#include <QtGui>

class GLWidget : public QGLWidget
{
    Q_OBJECT        // must include this if you use Qt signals/slots

public:
    IplImage *image;



public:
    GLWidget(QWidget *parent)
            : QGLWidget(parent), image(NULL) { setMinimumSize(300,300); }

    int iplToGl(IplImage *image, GLuint *tex) {
        if (image==NULL) return -1;

        glGenTextures(1, tex);
        glBindTexture( GL_TEXTURE_2D, *tex );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_BGR, GL_UNSIGNED_BYTE, image->imageData);
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        return 0;

    }

public slots:
    void changeImage()
    {
        if (image!=NULL) cvReleaseImage(&image);
        QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb", tr("Image Files (*.png *.jpg *.bmp)"));
        image =  cvLoadImage(fileName.toAscii()); //cvCreateImage(cvSize(400,400), IPL_DEPTH_8U, 3);


        cvLine(image, cvPoint(1,10), cvPoint(100,100), cvScalar(100,100,100), 1);
        cvRectangle(image, cvPoint(100,100), cvPoint(200,200), cvScalar(255,0,0), 1);


        unsigned int texImage;
        iplToGl(image, &texImage);
        glBindTexture( GL_TEXTURE_2D, texImage );

    }

protected:
    void initializeGL()
    {
        // Set up the rendering context, define display lists etc.:
        glEnable(GL_TEXTURE_2D);
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );

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
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
        glEnd();
    }
};

#endif // GLWIFGET_H
