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
    IplImage *frame1 = 0;
    IplImage* eig_image = 0;
    IplImage* temp_image = 0;

    if (image!=NULL) cvReleaseImage(&image);
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb", tr("Image Files (*.png *.jpg *.bmp)"));
    image =  cvLoadImage(fileName.toAscii());

    int w = image->width;
    int h = image->height;

    frame1 = cvCreateImage( cvSize(w, h), IPL_DEPTH_8U, 1);
    eig_image = cvCreateImage( cvSize(w, h), IPL_DEPTH_32F, 1);
    temp_image = cvCreateImage( cvSize(w, h), IPL_DEPTH_32F, 1);


    cvConvertImage( image, frame1 );

    int N = 1000;
    CvPoint2D32f frame1_features[N];

    cvGoodFeaturesToTrack(frame1, eig_image, temp_image, frame1_features, &N, .01, .01, NULL);

//    for( int i = 0; i < N; i++) {
//        int radius = w/25.0f;
//        cvCircle(image, cvPoint((int)(frame1_features[i].x + 0.5f),(int)(frame1_features[i].y + 0.5f)), radius,	cvScalar(0,255,0));
//    }

    cvScale(eig_image, eig_image, 100, 0.00);
    this->imageWidget->fromIpl(eig_image);

}

MainWindow::~MainWindow()
{
    delete ui;
}
