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
#include <fstream>
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

    int winSize = 12;
    int channels = 3;
    Samples samples;
    Dictionary dict(winSize, channels, 4096);
    TrainerMairal trainer;

//    dict.initRandom();
//    samples.loadImage(inputFilename, winSize, channels, 2);
//    trainer.train(samples, dict,  0, 10000);

    // for in qdir
//    QString epiPath = QString(inputPath.c_str());
//    QDir dir(epiPath);
    std::ifstream ifs( (inputPath + "list.txt").c_str() );
    int counter = 0;
//    foreach( Qtring name, dir.entryList(QStringList("*.png")) ) {
    std::string nameStr;  // = (epiPath + name).toStdString();
    while( !ifs.eof() ) {
        if(!running) break;
        ifs >> nameStr;
        std::cout << nameStr << std::endl;
        samples.loadImage(nameStr, winSize, channels);
        if(!counter) dict.initRandom();
//        if(!counter) dict.initFromData(samples);
        std::cout << "train set fill complete " << std::endl;
        if(!running) break;
        trainer.train(samples, dict,  0, 10000);
//        dict.debugSaveImage( (inputPath + "dict_tmp.png").c_str() );
        counter++;
    }
    ifs.close();

    trainer.save((inputPath + "simple.online.tmp").c_str() );
    dict.save( (inputPath + "simple.dict").c_str() );

    // sample image
    std::string outputFilename = inputPath + "lena_recon.jpg";
    Samples samples2;
    samples2.loadImage(inputFilename, winSize, channels, winSize);
    samples2.saveImage(outputFilename, dict );


}

