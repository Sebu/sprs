//#include <QtCore/QCoreApplication>

#include <libsparse/dictionary.h>
#include <libsparse/trainermairal.h>
#include <libsparse/coderlasso.h>
#include <libsparse/samples.h>
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

    std::string inputFilename = "/home/seb/Bilder/bild5.jpg";

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

    int winSize = 12;
    Samples samples;
    samples.load(inputFilename, winSize, 1);
    std::cout << "train set fill complete " << std::endl;

    Dictionary dict(winSize, 1, 400);
    dict.initRandom();
    //    dict.initFromData(training_set);

    TrainerMairal trainer;
    trainer.train(samples, dict, 200);

    dict.debugSaveImage("/home/seb/Bilder/dict_lasso.jpg");
    dict.save("/home/seb/Bilder/bla.dict");
    dict.load("/home/seb/Bilder/bla.dict");


//    cv::Mat outputImage(rowMax,colMax,CV_8UC(channels));
//    for(int j=0; j<rowMax/2; j+=size) {
//        for(int i=0; i<colMax/2; i+=size) {
//            index = ceil((float)j)*ceil((float)colMax) + ceil((float)i);
//            Matrix<double> signal = training_set.columnVector(index);
//            Matrix<double> a = lasso(signal, D);
//            Matrix<double> recon_vigra = D*a;
//            cv::Mat recon_cv(1,m,CV_8U);
//            for(int ii=0; ii<m; ii++)
//                recon_cv.at<uchar>(0,ii) = (uchar)(recon_vigra(ii,0)/scaling(0,index));
//            cv::Mat tmp = recon_cv.reshape(channels, size);
//            cv::Mat region( outputImage,  cv::Rect(i,j,size,size) );
//            tmp.copyTo(region);
//        }
//    }
//    cv::imwrite("/home/seb/Bilder/lena.recon.png", outputImage);


}
