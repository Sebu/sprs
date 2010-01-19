
#include <QGLWidget>
#include <fstream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/patch.h>
#include <epicore/cv_ext.h>


#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    debugWidgetL =  new AlbumWidget(ui->imageWidget);
    debugWidgetR =  new AlbumWidget(ui->otherWidget);

    this->connect( ui->loadButton, SIGNAL(clicked()),         this, SLOT(changeImage()) );
    this->connect( ui->saveButton, SIGNAL(clicked()),         this, SLOT(saveImage())   );
    this->connect( ui->calcButton, SIGNAL(clicked()),         this, SLOT(calculate())   );
    this->connect( ui->stepButton, SIGNAL(clicked()),         this, SLOT(step())        );
    this->connect( ui->prevButton, SIGNAL(clicked()), debugWidgetL, SLOT(prev())        );
    this->connect( ui->nextButton, SIGNAL(clicked()), debugWidgetL, SLOT(next())        );
    this->connect( debugWidgetL, SIGNAL(clicked(int,int)), this, SLOT(singleStep(int,int))      );


}



void MainWindow::changeImage() {

    fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb/Bilder", tr("Image Files (*.png *.jpeg *.jpg *.bmp)")).toStdString();
    if(fileName=="") return;

    image = cv::imread( fileName );
    debugWidgetL->fromIpl( image, "image" );
    this->seedmap = new SeedMap( image, ui->blockSpin->value(), ui->blockSpin->value(), ui->seedSpin->value(), ui->seedSpin->value());
    seedmap->maxError = ui->errorSpin->value();


    seedmap->loadMatches(fileName);


}

void MainWindow::step() {
    for (int i=0; i<ui->stepSpin->value(); i++)
        singleStep();
}


bool MainWindow::singleStep(int x, int y) {

    Patch* patch = 0;
    if(x==-1)
        patch = seedmap->matchNext();
    else {
        int block = ui->blockSpin->value();
        int xlocal = ((float)x/400.0) * (image.size().width / block);
        int ylocal = ((float)y/400.0) * (image.size().height / block);

        patch = seedmap->getPatch(xlocal,ylocal);
        seedmap->match(*patch);
    }

    if (!patch) return false;


#pragma omp critical
    {
        // debug
        if (!patch->matches->empty()) {

            cv::Mat tmpImage = seedmap->sourceImage.clone(); // patch->matches->front()->warp();

            // highlight block
            cv::rectangle(tmpImage, cv::Point(patch->x_, patch->y_),
                          cv::Point(patch->x_+patch->w_, patch->y_+patch->h_),
                          cv::Scalar(0,255,0,100),2);


            for(uint i=0; i<patch->matches->size(); i++) {
                std::vector<cv::Point> newPoints = patch->matches->at(i)->getMatchbox();

                // highlight match
                for(int i=0; i<4; i++){
                    cv::line(tmpImage, newPoints[i], newPoints[(i+1) % 4], cv::Scalar(0,0,255,100));
                }
                cv::line(tmpImage, newPoints[0], newPoints[1], cv::Scalar(255,0,0,100));

            }

            debugWidgetR->fromIpl( tmpImage, "preview" );
            cv::Mat reconstruction(seedmap->debugReconstruction());
            debugWidgetL->fromIpl( reconstruction, "reconstruction" );

            debugWidgetL->updateGL();
            debugWidgetR->updateGL();
        }

    }


    return true;
}

void MainWindow::calculate() {

    seedmap->resetMatches();

    //    #pragma omp parallel
    while(singleStep()) {}

    // FANCY DEBUG outputs
    cv::Mat reconstruction(seedmap->debugReconstruction());
    cv::Mat error( image.cols, image.rows, CV_8UC1);
    error = image - reconstruction;

    debugWidgetL->fromIpl( error, "error");

    debugWidgetL->update();

}

void MainWindow::saveImage() {
    // save demo reconstruction
    QString saveName = QFileDialog::getSaveFileName(this,tr("Save Image"), (fileName + ".recon.jpg").c_str(), tr("Image Files (*.png *.jpeg *.jpg *.bmp)"));
    seedmap->saveReconstruction(saveName.toStdString());

    // save matches
    seedmap->saveMatches(fileName);

}

MainWindow::~MainWindow()
{
    delete ui;
}
