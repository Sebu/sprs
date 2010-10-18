//#include <QtCore/QCoreApplication>

#include <libsparse/dictionary.h>
#include <libsparse/trainermairal.h>
#include <libsparse/coderlasso.h>
#include <libsparse/coderomp.h>
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

    std::string inputFilename = "/home/seb/Bilder/lena.png";

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
    int channels = 1;
    Samples samples;
    samples.load(inputFilename, winSize, channels);
    std::cout << "train set fill complete " << std::endl;

    Dictionary dict(winSize, 1, 225);
    dict.initRandom();
    //dict.initFromData(samples);

    TrainerMairal trainer;
    trainer.train(samples, dict,  10);

    dict.debugSaveImage("/home/seb/Bilder/dict_lasso.jpg");
    dict.save("/home/seb/Bilder/bla.dict");
//    dict.load("/home/seb/Bilder/bla.dict");

    CoderOMP coder;

    int m = winSize*winSize*channels;
    cv::Mat outputImage(samples.rowMax, samples.colMax, CV_8UC(channels));
    for(int j=0; j<samples.rowMax/2; j+=winSize) {
        for(int i=0; i<samples.colMax/2; i+=winSize) {
            int index = ceil((float)j)*ceil((float)samples.colMax) + ceil((float)i);
            Matrix<double> signal = samples.getData().columnVector(index);
            Matrix<double> a = coder.code(signal, dict);
            Matrix<double> recon_vigra = dict.getData()*a;
            cv::Mat recon_cv(1,m,CV_8U);
            for(int ii=0; ii<m; ii++)
                recon_cv.at<uchar>(0,ii) = (uchar)(recon_vigra(ii,0)/(*(samples.scaling_))(0,index));
            cv::Mat tmp = recon_cv.reshape(channels, winSize);
            cv::Mat region( outputImage,  cv::Rect(i,j,winSize, winSize) );
            tmp.copyTo(region);
            std::cout << "restore" << j*samples.rowMax+i << std::endl;
        }
    }
    cv::imwrite("/home/seb/Bilder/lena.recon.png", outputImage);


}
