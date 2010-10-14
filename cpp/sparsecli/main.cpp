//#include <QtCore/QCoreApplication>

#include <libsparse/dictionary.h>
#include <libsparse/regression.hxx>
#include <vigra/random.hxx>

#include <libsparse/vigra_ext.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>


using namespace vigra;
using namespace vigra::linalg;



int main(int argc, char *argv[])
{

    std::string inputFilename = "/homes/wheel/seb/Bilder/lena.png";

    int verbose = 0;
    int opt;

    while ((opt = getopt(argc, argv, "i:v")) != -1) {
        switch(opt) {
        case 'i':
            inputFilename = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case '?':
        default: /* '?' */
            std::cerr << "Usage: " << argv[0] << " [-i input_file]" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

        cv::Mat inputImage = cv::imread(inputFilename,0);

    int size = 8;
    int channels = 1;
    int m = size*size*channels;

    int rowMax = inputImage.rows;
    int colMax = inputImage.cols;
    int n = ceil((float)rowMax) * ceil((float)colMax);

    std::cout << n << std::endl;
    Matrix<double> training_set(m, n);


    int index = 0;
    for(int j=0; j<rowMax; j+=1) {
        for(int i=0; i<colMax; i+=1) {
            cv::Mat transMat = cv::Mat::eye(2,3,CV_64FC1);
            transMat.at<double>(0,2)=-i;
            transMat.at<double>(1,2)=-j;
            cv::Mat warped;
            cv::warpAffine(inputImage, warped, transMat, cv::Size(size, size));
            cv::Mat tmp = warped.reshape(1,1);
            std::cout << index << std::endl;
            for(int ii=0; ii<tmp.cols; ii++) {
                training_set(ii,index) = tmp.at<uchar>(0,ii);
            }

            index++;
        }
    }
    // normalize the input
    Matrix<double> offset(1,n), scaling(1,n);
    prepareColumns(training_set, training_set,offset, scaling, DataPreparationGoals(UnitNorm));

    std::cout << "train set fill complete " <<  index << std::endl;


    Dictionary dict(size,1, 400);

    dict.initRandom();
//    dict.initFromData(training_set);
    dict.learn(training_set, 200);
    dict.debugSaveImage("/homes/wheel/seb/Bilder/dict_lasso.jpg");

    Matrix<double> D = dict.getData();

    dict.save("/homes/wheel/seb/Bilder/bla.dict");
    dict.load("/homes/wheel/seb/Bilder/bla.dict");

    cv::Mat outputImage(rowMax,colMax,CV_8UC(channels));

    for(int j=0; j<rowMax/2; j+=size) {
        for(int i=0; i<colMax/2; i+=size) {
            index = ceil((float)j)*ceil((float)colMax) + ceil((float)i);
            Matrix<double> signal = training_set.columnVector(index);
            Matrix<double> a = lasso(signal, D);
            Matrix<double> recon_vigra = D*a;
            cv::Mat recon_cv(1,m,CV_8U);
            for(int ii=0; ii<m; ii++)
                recon_cv.at<uchar>(0,ii) = (uchar)(recon_vigra(ii,0)/scaling(0,index));
            cv::Mat tmp = recon_cv.reshape(channels, size);
            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
            tmp.copyTo(region);
        }
    }
    cv::imwrite("/homes/wheel/seb/Bilder/lena.recon.png", outputImage);

    //    std::cout << D << "b: " << signal << "a: " << a <<  std::endl;


}
