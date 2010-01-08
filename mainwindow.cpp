#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGLWidget>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "glwidget.h"
#include "patch.h"
#include "cv_ext.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imageWidget =  new AlbumWidget(ui->imageWidget);
    _otherWidget =  new AlbumWidget(ui->otherWidget);

    this->connect( ui->loadButton, SIGNAL(clicked()),         this, SLOT(changeImage()) );
    this->connect( ui->saveButton, SIGNAL(clicked()),         this, SLOT(saveImage()) );
    this->connect( ui->calcButton, SIGNAL(clicked()),         this, SLOT(calculate())  );
    this->connect( ui->stepButton, SIGNAL(clicked()),         this, SLOT(step())       );
    this->connect( ui->prevButton, SIGNAL(clicked()), imageWidget, SLOT(prev())        );
    this->connect( ui->nextButton, SIGNAL(clicked()), imageWidget, SLOT(next())        );



}



void MainWindow::changeImage() {

    fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb/Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));
    if(fileName=="") return;


    image = cv::imread( fileName.toStdString() );
    imageWidget->fromIpl( image, "image" );

}

void MainWindow::step() {
    for (int i=0; i<ui->stepSpin->value(); i++)
        singleStep();
}

bool MainWindow::singleStep() {
    static int x = 0;
    static int y = 0;
    static int maxX, maxY;

    if(x==0 && y==0) {

        this->seedmap = new SeedMap( image, ui->blockSpin->value(), ui->blockSpin->value(), ui->seedSpin->value(), ui->seedSpin->value());
        seedmap->debugAlbum = this->imageWidget;
        seedmap->debugAlbumR = this->_otherWidget;
        seedmap->maxError = ui->errorSpin->value();

        int w = ui->blockSpin->value();
        int h = ui->blockSpin->value();

        maxX = (image.cols / w);
        maxY = (image.rows / h);

    }

    seedmap->testPatch(x,y);

    x++;
    if(x>=maxX) { x=0; y++; }
    if(y>=maxY) { y=0; return false; }

    return true;
}

void MainWindow::calculate() {
    calculateTimed();
}

void MainWindow::calculateTimed() {


    while(singleStep()) {}

    // FANCY DEBUG outputs
    cv::Mat reconstruction(seedmap->reconstructIpl());
    cv::Mat error( image.cols, image.rows, CV_8UC1);
    error = image - reconstruction;

    imageWidget->fromIpl( error,          "error");

    imageWidget->update();

}

void MainWindow::saveImage() {

    //    QString name = fileName;
    //    std::string saveName = "reconstruction" + name.remove(0, name.lastIndexOf("/")).toStdString();

    QString saveName = QFileDialog::getSaveFileName(this,tr("Save Image"), "reconstruction/", tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));
    cv::Mat reconstruction(seedmap->reconstructIpl());
    std::cout << saveName.toStdString() << std::endl;
    cv::imwrite(saveName.toStdString(), reconstruction);
}

MainWindow::~MainWindow()
{
    delete ui;
}
