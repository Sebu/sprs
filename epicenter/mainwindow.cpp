
#include <QGLWidget>
#include <fstream>
#include <opencv/cv.h>
#include <opencv/highgui.h>


#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    debugWidgetL =  new AlbumWidget(ui->imageWidget);
    debugWidgetR =  new AlbumWidget(ui->otherWidget);

    calcThread.debugWidgetL = debugWidgetL;
    calcThread.debugWidgetR = debugWidgetR;

    this->connect( ui->loadButton, SIGNAL(clicked()),         this, SLOT(changeImage()) );
    this->connect( ui->baseButton, SIGNAL(clicked()),         this, SLOT(addBase()) );

    this->connect( ui->saveButton, SIGNAL(clicked()),         this, SLOT(saveImage())   );
    this->connect( ui->prevButton, SIGNAL(clicked()), debugWidgetL, SLOT(prev())        );
    this->connect( ui->nextButton, SIGNAL(clicked()), debugWidgetL, SLOT(next())        );
    this->connect( debugWidgetL, SIGNAL(clicked(int,int)), &calcThread, SLOT(singleStep(int,int))      );
    this->connect( ui->stepButton, SIGNAL(clicked()),         &calcThread, SLOT(step())        );
    this->connect( ui->step2Button, SIGNAL(clicked()),        &calcThread, SLOT(step2()) );
    this->connect( ui->calcButton, SIGNAL(clicked()),         &calcThread, SLOT(calculate())   );


}


void MainWindow::addBase() {

    std::string bName = QFileDialog::getOpenFileName(this,tr("Open Image"), "../../../Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)")).toStdString();
    if(bName=="") return;
    calcThread.searchInOriginal_ = false;
    calcThread.base_ = cv::imread( bName );

    // TODO: clear seeds, add news seeds

    if(calcThread.seedmap)
        calcThread.seedmap->setReconSource(calcThread.base_,1);
}


void MainWindow::changeImage() {

    fileName_ = QFileDialog::getOpenFileName(this,tr("Open Image"), "../../../Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)")).toStdString();
    if(fileName_=="") return;

    if(calcThread.seedmap) delete calcThread.seedmap;
    calcThread.seedmap = 0;
    calcThread.fileName = fileName_;
    calcThread.image_ = cv::imread( fileName_ );
    calcThread.base_ = calcThread.image_;
    calcThread.searchInOriginal_ = true;
    debugWidgetL->fromIpl( calcThread.image_, "image" );
    calcThread.blockSize_ = ui->blockSpin->value();
    calcThread.error_  = ui->errorSpin->value();

}



void MainWindow::saveImage() {
    // save demo reconstruction
//    QString saveName = QFileDialog::getSaveFileName(this,tr("Save Image"), (fileName_ + ".recon.jpg").c_str(), tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));
    // save matches
    calcThread.seedmap->serialize(fileName_);
    calcThread.seedmap->saveReconstruction(fileName_); //saveName.toStdString());
    calcThread.seedmap->saveCompressedImage(fileName_);
    calcThread.seedmap->saveEpitome(fileName_);

}

MainWindow::~MainWindow()
{
    delete ui;
}
