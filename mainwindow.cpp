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
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb/Bilder", tr("Image Files (*.png *.jpg *.bmp)"));
    _image =  cvLoadImage(fileName.toAscii());

    int w = 16;
    int h = 16;

    IplImage* gray = cvCreateImage( cvSize(_image->width, _image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(_image, gray, CV_BGR2GRAY);


    Patch* patch1 = new Patch( sub_image(gray, cvRect(0,0,w,h)) );


 /*
    int N = 400;
    CvPoint2D32f frame1_features[N];
    eig_image = cvCreateImage( cvSize(w, h), IPL_DEPTH_32F, 1);
    temp_image = cvCreateImage( cvSize(w, h), IPL_DEPTH_32F, 1);
    cvGoodFeaturesToTrack(patch1, eig_image, temp_image, frame1_features, &N, .01, .01, NULL);


    IplImage* py1 = cvCreateImage( cvSize(w, h), IPL_DEPTH_8U, 1);
    IplImage* py2 = cvCreateImage( cvSize(w, h), IPL_DEPTH_8U, 1);

    CvPoint2D32f frame2_features[N];
    char optical_flow_found_feature[N];
    float optical_flow_feature_error[N];
    CvSize optical_flow_window = cvSize(3,3);
    CvTermCriteria optical_flow_termination_criteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );

    cvCalcOpticalFlowPyrLK( patch1, patch2, py1, py2, frame1_features,
                            frame2_features, N, optical_flow_window, 5,
                            optical_flow_found_feature, optical_flow_feature_error,
                            optical_flow_termination_criteria, 0 );

    for( int i = 0; i < N; i++) {
        int radius = w/25.0f;
        cvCircle(image, cvPoint((int)(frame1_features[i].x + 0.5f),(int)(frame1_features[i].y  + 0.5f)), radius,	cvScalar(255,0,0));
        cvCircle(gray, cvPoint((int)(frame2_features[i].x + offsetx + 0.5f),(int)(frame2_features[i].y + offsety + 0.5f)), radius,	cvScalar(255,0,0));

    }
*/

    this->_seedmap = new SeedMap(_image,4,4);

    this->_imageWidget->fromIpl(_image , "gray");
    this->_imageWidget->fromIpl( _seedmap->meanIpl(), "seeds histogram means" );
    this->_imageWidget->fromIpl( _seedmap->orientIpl(patch1->_orientHist.peak()), "seeds orientation" );

}




MainWindow::~MainWindow()
{
    delete ui;
}
