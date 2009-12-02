#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGLWidget>
#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), image(NULL), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imageWidget =  new GLWidget(ui->imageWidget);
    otherWidget =  new GLWidget(ui->otherWidget);

    this->connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(changeImage()));




}

IplImage* sub_image(IplImage *image, CvRect roi)
{
    IplImage *result;
    // set ROI, you may use following two funs:
    //cvSetImageROI( image, cvRect( 0, 0, image->width, image->height ));

    cvSetImageROI(image,roi);
    // sub-image
    result = cvCreateImage( cvSize(roi.width, roi.height), image->depth, image->nChannels );
    cvCopy(image,result);
    cvResetImageROI(image); // release image ROI
    return result;
}

float histogram_mean(IplImage* img) {
    float mean= 0.0f;

    /*
    int hist_size = 256;

    float s_ranges[] = { 0, 255 };
    float* ranges[] = { s_ranges };

    IplImage* red = cvCreateImage( cvSize(img->width, img->height), img->depth, 1 );

    cvSplit(img, red, NULL, NULL, NULL);

    CvHistogram* hist = cvCreateHist( 1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    cvCalcHist( &img, hist, 0, 0 );

    for(int i = 0; i < hist_size; i++ ) {
        float* bins = cvGetHistValue_1D(hist,i);
        mean += bins[0];
        //printf("%f ", bins[0]);
    }
    //printf("\n");
    cvReleaseHist(&hist);
    */
    mean = cvAvg(img).val[0]/256.0f; // /= (float)hist_size;

    printf("%f \n", mean);
    return mean;
}

void MainWindow::changeImage()
{

    if (image!=NULL) cvReleaseImage(&image);
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "/home/seb", tr("Image Files (*.png *.jpg *.bmp)"));
    image =  cvLoadImage(fileName.toAscii());

    int w = 16; //image->width;
    int h = 16; //image->height;
    int offsetx = 16;
    int offsety = 16;

    IplImage* gray = cvCreateImage( cvSize(image->width, image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(image, gray, CV_BGR2GRAY);
    //cvConvertImage( image, gray );

    IplImage* patch1 = sub_image(gray, cvRect(0,0,w,h));
    IplImage* patch2 = sub_image(gray, cvRect(offsetx,offsety,w,h));



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
    IplImage* mean_dist = cvCreateImage( cvSize(image->width-w, image->height-h), IPL_DEPTH_32F, 1);

    float patch_mean = histogram_mean(patch1);


    for(int x=0; x<image->width-w; x+=w/4){
      for(int y=0; y<image->height-h; y+=h/4){
        IplImage* patch = sub_image(gray, cvRect(x,y,w,h));
        float mean = patch_mean / histogram_mean(patch);
        //printf("%i, %i, %f\n",x, y, mean);
        cvRectangle(mean_dist, cvPoint(x/4,y/4), cvPoint(x/4,y/4), cvScalarAll(mean), 1);
      }
    }



    this->imageWidget->fromIpl(image);
    this->otherWidget->fromIpl(mean_dist);

}




MainWindow::~MainWindow()
{
    delete ui;
}
