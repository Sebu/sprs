#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGLWidget>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "glwidget.h"
#include "patch.h"
#include "cv_ext.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _image(NULL), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _imageWidget =  new AlbumWidget(ui->imageWidget);

    this->connect( ui->pushButton, SIGNAL(clicked()),         this, SLOT(changeImage()) );
    this->connect( ui->prevButton, SIGNAL(clicked()), _imageWidget, SLOT(prev())        );
    this->connect( ui->nextButton, SIGNAL(clicked()), _imageWidget, SLOT(next())        );



}





void MainWindow::changeImage()
{

    if (_image!=NULL) cvReleaseImage(&_image);

    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb/Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));

    if(fileName=="") return;

    _image =  cvLoadImage(fileName.toAscii());

    int w = ui->blockSpin->value();
    int h = ui->blockSpin->value();

    IplImage* gray = cvCreateImage( cvSize(_image->width, _image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(_image, gray, CV_BGR2GRAY);



    this->_seedmap = new SeedMap( gray,  w, h, ui->stepSpin->value(), ui->stepSpin->value());

    int maxX = gray->width  / w;
    int maxY = gray->height / h;

    for (int y=0; y<maxY; y++){
        for(int x=0; x<maxX; x++) {
            Patch* patch = new Patch( gray, x*w, y*h, w, h );
            patch->_debugAlbum = this->_imageWidget;
            this->_seedmap->match(*patch);
            free(patch);
        }
    }

    IplImage* reconstruction = _seedmap->reconstructIpl();
    IplImage* error = cvCreateImage( cvSize(_image->width, _image->height), IPL_DEPTH_8U, 1);
    cvSub(gray,reconstruction,error);


    _imageWidget->fromIpl( _image,                        "image" );
//    _imageWidget->fromIpl( _seedmap->meanIpl(),           "seeds histogram means" );
//    _imageWidget->fromIpl( _seedmap->orientIpl(),         "seeds orientation" );
    _imageWidget->fromIpl( _seedmap->epitomeIpl() ,       "test epitome" );
    _imageWidget->fromIpl( error,                          "error");
    _imageWidget->fromIpl( gray,                          "gray");
    _imageWidget->fromIpl( reconstruction,                "reconstruction" );


}




MainWindow::~MainWindow()
{
    delete ui;
}
