
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

    std::string bla = QFileDialog::getOpenFileName(this,tr("Open Image"), "../../../Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)")).toStdString();
    if(bla=="") return;
    calcThread.searchInOriginal_ = false;
    calcThread.base = cv::imread( bla );

    // TODO: clear seeds, add news seeds

    if(calcThread.seedmap)
        calcThread.seedmap->setReconSource(calcThread.base,1);
}


void MainWindow::changeImage() {

    fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "../../../Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)")).toStdString();
    if(fileName=="") return;

    if(calcThread.seedmap) delete calcThread.seedmap;
    calcThread.seedmap = 0;
    calcThread.fileName = fileName;
    calcThread.image = cv::imread( fileName );
    calcThread.base = calcThread.image;
    calcThread.searchInOriginal_ = true;
    debugWidgetL->fromIpl( calcThread.image, "image" );
    calcThread.blockSize = ui->blockSpin->value();
    calcThread.error  = ui->errorSpin->value();

}



void MainWindow::saveImage() {
    // save demo reconstruction
    QString saveName = QFileDialog::getSaveFileName(this,tr("Save Image"), (fileName + ".recon.jpg").c_str(), tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));
    calcThread.seedmap->saveReconstruction(saveName.toStdString());
    // save matches
    calcThread.seedmap->serialize(fileName);

}

MainWindow::~MainWindow()
{
    delete ui;
}
