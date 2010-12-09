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

    int verbose = 0;
    int opt = 0;

    std::string inputFile = "";
    std::string testFile = "";

    int sampleCount = 10000;
    int dictSize = 4096;
    double eps = 0.0;
    int  coeffs = 20;

    while ((opt = getopt(argc, argv, "d:i:s:e:c:t:v")) != -1) {
        switch(opt) {
        case 'd':
            dictSize = atoi(optarg);
            break;
        case 't':
            coeffs = atoi(optarg);
            break;
        case 'e':
            eps = (double)atof(optarg);
            break;
        case 'i':
            inputFile = optarg;
            break;
        case 's':
            sampleCount = atoi(optarg);
            break;
        case 'c':
            testFile = optarg;
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


    int winSize = 8;
    int channels = 3;
    Dictionary dict(winSize, channels, dictSize);


    if(inputFile != "")
    {
        Samples samples;
        TrainerMairal trainer;
        std::ifstream ifs( (inputFile).c_str() );
        int counter = 0;
        std::string nameStr;
        ifs >> nameStr;
        while( !ifs.eof() ) {
            if(!running) break;
            std::cout << nameStr << std::endl;
            samples.loadImage(nameStr, winSize, channels);
            if(!counter) { dict.initRandom(); }
    //        if(!counter) dict.initFromData(samples);
            std::cout << "train set fill complete " << std::endl;
            if(!running) break;
            trainer.train(samples, dict,  0, sampleCount);
            counter++;
            ifs >> nameStr;
        }
        ifs.close();
        dict.debugSaveImage( (inputFile + ".dict.png").c_str() );
        dict.save( (inputFile + ".dict").c_str() );
    }
    //    trainer.save((inputFile + ".tmp").c_str() );


    if(testFile!="")
    {
        Samples samples2;
        std::string outputFilename = testFile + ".recon.jpg";
        //dict.load( (inputFile + ".dict").c_str() );
        dict.initRandom();
        samples2.loadImage(testFile, winSize, channels, winSize);
        samples2.saveImage(outputFilename, dict );
    }

}

