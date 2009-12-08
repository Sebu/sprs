#include "patch.h"
#include "cv_ext.h"


bool Patch::match(Patch& other) {


    float colorScale = this->histMean() - other.histMean();

    if(colorScale>1.25f) return false;

//    float orientationError = this->_orientHist->compare(other._orientHist);

    IplImage* diff = cvCreateImage( cvSize(this->_w, this->_h),
                                    this->_patchImage->depth, this->_patchImage->nChannels );

    IplImage* mul = cvCreateImage( cvSize(this->_w, this->_h),
                                    this->_patchImage->depth, this->_patchImage->nChannels );

    cvAbsDiff(this->_patchImage, other._patchImage, diff);

    float scaleFactor = 1.0 / (255.0*255.0);
    cvMul(diff, diff, mul, scaleFactor);


    float alpha = 1.0f; // 0 <= alpha <= 2
    float beta  = .2f;

    // reconError =  ( cvSum(mul).val[0] ) / ( pow( variance , alpha) + beta );
    float reconError =  ( (float)cvSum(mul).val[0] ) /  beta ;

    cvReleaseImage(&diff);
    cvReleaseImage(&mul);

    std::cout << reconError << std::endl;

    //   TODO: magic number
    return (reconError<1.1f);
    //return 0;


    int N = 10;
    CvPoint2D32f frame1_features[N];
    IplImage* eig_image = cvCreateImage( cvSize(_w, _h), IPL_DEPTH_32F, 1);
    IplImage* temp_image = cvCreateImage( cvSize(_w, _h), IPL_DEPTH_32F, 1);

    cvGoodFeaturesToTrack(this->_patchImage, eig_image, temp_image, frame1_features, &N, .01, .01, NULL);


    CvPoint2D32f frame2_features[N];
    char optical_flow_found_feature[N];
    float optical_flow_feature_error[N];
    CvSize optical_flow_window = cvSize(3,3);
    CvTermCriteria optical_flow_termination_criteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );
    IplImage* py1 = cvCreateImage( cvSize(_w, _h), IPL_DEPTH_8U, 1);
    IplImage* py2 = cvCreateImage( cvSize(_w, _h), IPL_DEPTH_8U, 1);

    cvCalcOpticalFlowPyrLK( this->_patchImage, other._patchImage, py1, py2, frame1_features,
                            frame2_features, N, optical_flow_window, 5,
                            optical_flow_found_feature, optical_flow_feature_error,
                            optical_flow_termination_criteria, 0 );

    cvReleaseImage(&py1);
    cvReleaseImage(&py2);
    cvReleaseImage(&eig_image);
    cvReleaseImage(&temp_image);

    for( int i = 0; i < N; i++) {
//        int radius = _w/25.0f;
//        cvCircle(image, cvPoint((int)(frame1_features[i].x + 0.5f),(int)(frame1_features[i].y  + 0.5f)), radius,	cvScalar(255,0,0));
//        cvCircle(gray, cvPoint((int)(frame2_features[i].x + offsetx + 0.5f),(int)(frame2_features[i].y + offsety + 0.5f)), radius,	cvScalar(255,0,0));

    }

}

Patch::Patch(IplImage* sourceImage, int x, int  y, int w, int h):
        _histMean(0.0f), _x(x), _y(y), _w(w), _h(h), _patchImage(0), _sourceImage(sourceImage)
{

    _patchImage = sub_image( sourceImage, cvRect(_x,_y,_w,_h) );
    _orientHist = new OrientHist(_patchImage, 36);
    setHistMean( histogram_mean(_patchImage) );
}
