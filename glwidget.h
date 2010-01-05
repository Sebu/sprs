#ifndef GLWIFGET_H
#define GLWIFGET_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QGLWidget>
#include <QtGui>

class AlbumWidget : public QGLWidget
{
Q_OBJECT

public:


    AlbumWidget(QWidget *parent)
            : QGLWidget(parent), _pos(0) {

        setMinimumSize(400,400);
    }

    int fromIpl(cv::Mat& image, QString caption="none");

public slots:
    void next();
    void prev();

protected:
    QHash<QString, unsigned int> _texImages;
   QList<unsigned int>  _values;
    int _pos;

    void validatePos();

    void initializeGL()
    {
        glEnable(GL_TEXTURE_2D);
        glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
    }

    void resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);
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
