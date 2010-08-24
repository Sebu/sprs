#include "glwidget.h"

#include <iostream>
#include <X11/Xlib.h>

void AlbumWidget::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked(event->pos().x(), event->pos().y());
}



void AlbumWidget::prev() {
    _pos--;

    makeCurrent();
    unsigned int tex = _values.at(_pos % _values.size());
    glBindTexture( GL_TEXTURE_2D, tex );
    update();
}

void AlbumWidget::next() {
    _pos++;

    makeCurrent();
    unsigned int tex = _values.at(_pos % _values.size());
    glBindTexture( GL_TEXTURE_2D, tex );
    update();
}

int AlbumWidget::fromIpl(cv::Mat& image, QString caption) {

    makeCurrent();

    unsigned int tex;

    if (!_texImages.contains(caption)) {
        glGenTextures(1, &tex);
    } else {
        tex = _texImages[caption];
    }
    _texImages[caption] = tex;

    _values = _texImages.values();
     _pos = _values.size();

    glBindTexture( GL_TEXTURE_2D, tex );


    GLenum input_format;

    switch(image.channels()) {
        case 1: input_format = GL_LUMINANCE; break;
        case 3: input_format = GL_BGR; break;
        case 4: input_format = GL_BGRA; break;
        default: input_format = GL_BGR; break;
    }

    GLenum channel_depth;

    switch(image.depth()) {
        case CV_8U: channel_depth  = GL_UNSIGNED_BYTE; break;
        case CV_32F: channel_depth = GL_FLOAT; break;
        default: channel_depth = GL_UNSIGNED_BYTE; break;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, input_format, channel_depth, image.data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    return 0;

}

