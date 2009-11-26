#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGLWidget>
#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), image(NULL), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imageWidget =  new GLWidget(ui->imageWidget);

    this->connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(changeImage()));




}
void MainWindow::changeImage()
{
    if (image!=NULL) cvReleaseImage(&image);
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb", tr("Image Files (*.png *.jpg *.bmp)"));
    image =  cvLoadImage(fileName.toAscii());

    this->imageWidget->fromIpl(image);

}

MainWindow::~MainWindow()
{
    delete ui;
}
