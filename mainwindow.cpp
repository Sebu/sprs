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
    _otherWidget =  new AlbumWidget(ui->otherWidget);

    this->connect( ui->loadButton, SIGNAL(clicked()),         this, SLOT(changeImage()) );
    this->connect( ui->calcButton, SIGNAL(clicked()),         this, SLOT(calculate())   );
    this->connect( ui->stepButton, SIGNAL(clicked()),         this, SLOT(singleStep())  );
    this->connect( ui->prevButton, SIGNAL(clicked()), _imageWidget, SLOT(prev())        );
    this->connect( ui->nextButton, SIGNAL(clicked()), _imageWidget, SLOT(next())        );



}



void MainWindow::changeImage() {
    if (_image) cvReleaseImage(&_image);
    _fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb/Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));
    if(_fileName=="") return;

    _image = cvLoadImage(_fileName.toAscii());
    _gray = cvCreateImage( cvSize(_image->width, _image->height), IPL_DEPTH_8U, 1);

    cvCvtColor(_image, _gray, CV_BGR2GRAY);


}

void MainWindow::singleStep() {
    static int x = 0;
    static int y = 0;
    static int maxX, maxY;

    if(x==0 && y==0) {
        this->_seedmap = new SeedMap( _gray, ui->blockSpin->value(), ui->blockSpin->value(), ui->stepSpin->value(), ui->stepSpin->value());
        _seedmap->_debugAlbum = this->_imageWidget;
        _seedmap->_debugAlbumR = this->_otherWidget;
        _seedmap->_error = ui->errorSpin->value();

        int w = ui->blockSpin->value();
        int h = ui->blockSpin->value();

        maxX = _image->width  / w;
        maxY = _image->height / h;
        std::cout << "once" << maxX << maxY << std::endl;
    }

    _seedmap->testPatch(x,y);

    x++;
    if(x>=maxX) { x=0; y++; }
    if(y>=maxY) { y=0; }


}

void MainWindow::calculate() {
    if(!_image) return;

    this->_seedmap = new SeedMap( _gray, ui->blockSpin->value(), ui->blockSpin->value(), ui->stepSpin->value(), ui->stepSpin->value());
    _seedmap->_debugAlbum = this->_imageWidget;
    _seedmap->_debugAlbumR = this->_otherWidget;
    _seedmap->_error = ui->errorSpin->value();

    int w = ui->blockSpin->value();
    int h = ui->blockSpin->value();

    int maxX = _image->width  / w;
    int maxY = _image->height / h;
    std::cout << "once" << maxX << maxY << std::endl;

    for (int y=0; y<maxY; y++){
        for(int x=0; x<maxX; x++) {
            _seedmap->testPatch(x,y);
        }
    }



    // FANCY DEBUG outputs
    IplImage* reconstruction = _seedmap->reconstructIpl();
    IplImage* error = cvCreateImage( cvSize(_image->width, _image->height), IPL_DEPTH_8U, 1);
    cvSub(_gray, reconstruction, error);

    _imageWidget->fromIpl( _image,                        "image" );
//    _imageWidget->fromIpl( _seedmap->meanIpl(),           "seeds histogram means" );
//    _imageWidget->fromIpl( _seedmap->orientIpl(),         "seeds orientation" );
//    _imageWidget->fromIpl( _seedmap->epitomeIpl() ,       "test epitome" );
    _imageWidget->fromIpl( error,                          "error");
    _imageWidget->fromIpl( _gray,                          "gray");
    _imageWidget->fromIpl( reconstruction,                "reconstruction" );



    // save image :)
    QString name = _fileName;
    const char* saveName = "reconstruction" + name.remove(0, name.lastIndexOf("/")).toAscii();
    std::cout << saveName << std::endl;
    cvSaveImage(saveName, reconstruction);
    _imageWidget->update();


    // cleanup
    cvReleaseImage(&reconstruction);
    cvReleaseImage(&error);
}




MainWindow::~MainWindow()
{
    delete ui;
}
