//#include <QtCore/QCoreApplication>

#include <libsparse/dictionary.h>
#include <libsparse/trainermairal.h>
#include <libsparse/coderlasso.h>
#include <libsparse/coderomp.h>
#include <libsparse/samples.h>


#include <iostream>
#include <fstream>
#include <QDir>
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

    int opt = 0;

    bool verbose = false;
    bool resume = false;
    std::string dictFile = "";
    std::string trainFile = "";
    std::string imageFile = "";
    int sampleCount = 10000;
    int dictSize = 4096;
    double eps = 0.0;
    int  coeffs = 20;
    int winSize = 8;
    int channels = 3;
    int mode = 1;

    while ((opt = getopt(argc, argv, "c:d:e:f:i:m:rs:t:vw:")) != -1) {
        switch(opt) {
        case 'c':
            coeffs = atoi(optarg);
            break;
        case 'd':
            dictSize = atoi(optarg);
            break;
        case 'e':
            eps = (double)atof(optarg);
            break;
        case 'f':
            dictFile = optarg;
            break;
        case 'i':
            imageFile = optarg;
            break;
        case 'm':
            mode = atoi(optarg);
            break;
        case 'r':
            resume = true;
            break;
        case 's':
            sampleCount = atoi(optarg);
            break;
        case 't':
            trainFile = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'w':
            winSize = atoi(optarg);
            break;
        case '?':
        default: /* '?' */
            std::cerr << "Usage: " << argv[0] << " [-i image_file] [-t train_file] [-d dict_size] [-s sample_count] [-c coefficents] [-e epsilon]"  << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    for(int i=1; i<argc; i++)
        std::cout << argv[i] << " ";
    std::cout << std::endl;

    Dictionary dict(winSize, channels, dictSize);

    Coder* coder = 0;
    switch(mode) {
    case 1: coder = new CoderOMP(); break;
    case 2: coder = new CoderLasso(); break;
    }

    coder->eps = eps;
    coder->coeffs = coeffs;



    if(trainFile != "")
    {
        Samples samples;
        TrainerMairal trainer;
        trainer.coder = coder;

        if(resume) {
            dict.load(dictFile.c_str());
            trainer.load((dictFile + ".tmp").c_str() );
        } else {
            dict.initRandom();
        }

        std::ifstream ifs( (trainFile).c_str() );
        std::string nameStr;
        ifs >> nameStr;
        int counter=0;
        while( !ifs.eof() ) {
            if(!running) break;
            std::cout << nameStr << " " << counter++ << std::endl;
            samples.loadImage(nameStr, winSize, channels, 1);
//            if(!resume && counter==1) dict.initFromData(samples);
            std::cout << "train set fill complete " << std::endl;
            if(!running) break;
            trainer.train(samples, dict,  0, sampleCount);
            ifs >> nameStr;
        }
        ifs.close();
        //dict.debugSaveImage( (dictFile + ".png").c_str() );
        dict.save( dictFile.c_str() );
        trainer.save((dictFile + ".tmp").c_str() );
    }


    if(imageFile!="")
    {
        Samples samples;
        std::string outputFilename = imageFile + ".recon2.jpg";
        dict.load( dictFile.c_str() );
        //dict.initRandom();
        samples.loadImage(imageFile, winSize, channels, winSize);
        std::cout << "worked" << std::endl;
        samples.saveImage(outputFilename, dict, *coder);

    }

}

