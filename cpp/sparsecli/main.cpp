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


    int verbose = 0;
    int opt;


    std::string inputPath;
    while ((opt = getopt(argc, argv, "i:v")) != -1) {
        switch(opt) {
        case 'i':
            inputPath = optarg;
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

    std::string inputFilename = inputPath +  "lena_color.jpg";

    int winSize = 8;
    int channels = 3;
    Samples samples;
    samples.load(inputFilename, winSize, channels);
    std::cout << "train set fill complete " << std::endl;

    Dictionary dict(winSize, channels, 225);
    dict.initRandom();
//    dict.initFromData(samples);

    TrainerMairal trainer;
    trainer.train(samples, dict,  30);

    dict.save( (inputPath + "simple.dict").c_str() );
    dict.load( (inputPath + "simple.dict").c_str() );
    dict.debugSaveImage( (inputPath + "dict.png").c_str() );

    CoderLasso coder;

    int m = winSize*winSize*channels;

    std::cout << "restore image" << std::endl;
    cv::Mat outputImage(samples.rows_, samples.cols_, CV_8UC(channels));
    for(int j=0; j<samples.rows_; j+=winSize) {
        for(int i=0; i<samples.cols_; i+=winSize) {
            int index = ceil((float)j)*ceil((float)samples.cols_) + ceil((float)i);
            Matrix<double> signal = samples.getData().columnVector(index);
            Matrix<double> a = coder.code(signal, dict);
            Matrix<double> recon_vigra = dict.getData()*a;
            cv::Mat recon_cv(1,m,CV_8U);
            for(int ii=0; ii<m; ii++)
                recon_cv.at<uchar>(0,ii) = cv::saturate_cast<uchar>(recon_vigra(ii,0)/(*(samples.scaling_))(0,index));
            cv::Mat tmp = recon_cv.reshape(channels, winSize);
            cv::Mat region( outputImage,  cv::Rect(i,j,winSize, winSize) );
            tmp.copyTo(region);
//            std::cout << "restore" << j*samples.rows_+i << std::endl;
        }
    }
    cv::imwrite( inputPath + "lena_recon.jpg", outputImage);


}
