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
#include <QDir>

using namespace vigra;
using namespace vigra::linalg;

#include <signal.h>
bool running = true;

void terminate(int param)
{
    running = false;
}

int main(int argc, char *argv[])
{

    void (*prev_fn)(int);

    prev_fn = signal(SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal(SIGINT,SIG_IGN);
    prev_fn = signal (SIGTERM,terminate);
    if (prev_fn==SIG_IGN) signal(SIGTERM,SIG_IGN);
    prev_fn = signal (SIGTERM,terminate);
    if (prev_fn==SIG_IGN) signal(SIGKILL,SIG_IGN);

    int verbose = 0;
    int opt;


    std::string inputPath = "";
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
    samples.loadImage(inputFilename, winSize, channels);
    Dictionary dict(winSize, channels, 4096);
    dict.initRandom();
//    dict.initFromData(samples);
    TrainerMairal trainer;
    trainer.train(samples, dict,  5, 1000);
    dict.debugSaveImage( (inputPath + "dict.png").c_str() );

//    // for in qdir
//    QString epiPath = QString(inputPath.c_str());
//    QDir dir(epiPath);
////    std::ifstream ifs( input );
//    int counter = 0;
//    foreach( QString name, dir.entryList(QStringList("*.png")) ) {
//        if(!running) break;
//        std::string nameStr = (epiPath + name).toStdString();
//        std::cout << nameStr << std::endl;
//        samples.loadImage(nameStr, winSize, channels);
//        std::cout << "train set fill complete " << std::endl;
//        if(!running) break;
//        trainer.train(samples, dict,  3, 1000);
//        dict.debugSaveImage( (inputPath + "dict_tmp.png").c_str() );
//        counter++;
//    }


//    trainer.save((inputPath + "simple.tmpdict").c_str() );
////    trainer.load((inputPath + "simple.tmpdict").c_str() );
////    trainer.train(samples, dict,  10);
//    dict.save( (inputPath + "simple.dict").c_str() );
////    dict.load( (inputPath + "simple.dict").c_str() );
////    dict.debugSaveImage( (inputPath + "dict.png").c_str() );

//    // sample image
////    std::string inputFilename = inputPath +  "stallone460klein.jpg";
////    std::string outputFilename = inputPath + "lena_recon.jpg";
////    Samples samples2;
////    samples2.loadImage(inputFilename, winSize, channels, winSize);
////    samples2.saveImage(outputFilename, dict );


}

