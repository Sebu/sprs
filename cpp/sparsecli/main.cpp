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
    Samples samples; // (winSize, channels)
    samples.load(inputFilename, winSize, channels);
    std::cout << "train set fill complete " << std::endl;

    Dictionary dict(winSize, channels, 225);
    dict.initRandom();
//    dict.initFromData(samples);

    TrainerMairal trainer;
    trainer.train(samples, dict,  10);

    dict.save( (inputPath + "simple.dict").c_str() );
    dict.load( (inputPath + "simple.dict").c_str() );
    dict.debugSaveImage( (inputPath + "dict.png").c_str() );

    std::string outputFilename = inputPath + "lena_recon.jpg";
    samples.save( outputFilename , dict );


}
