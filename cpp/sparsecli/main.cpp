//#include <QtCore/QCoreApplication>

#include <sprscode/dictionary.h>
#include <sprscode/trainermairal.h>
#include <sprscode/coderlasso.h>
#include <sprscode/coderomp.h>
#include <sprscode/samples.h>


#include <iostream>
#include <fstream>
#include <QDir>
#include <signal.h>
#include <getopt.h>

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

    int verbose = 0;
    int resume = 0;
    std::string dictFile = "";
    std::string trainFile = "";
    std::string mergeFile = "";
    std::string imageFile = "";
    int sampleCount = 10000;
    int dictSize = 4096;
    double eps = 0.0;
    int  coeffs = 20;
    int blockSize = 8;
    int winSize = 8;
    int channels = 3;
    int mode = 1;


    static struct option long_options[] =
    {
        /* These options set a flag. */
        {"blockSize",    required_argument, 0, 'b'},
        {"winSize",    required_argument, 0, 'w'},
        {"coeffs",    required_argument, 0, 'c'},
        {"channels", required_argument, 0, 'l'},
        {"dictSize",  required_argument, 0, 'd'},
        {"epsilon",    required_argument, 0, 'e'},
        {"dict",    required_argument, 0, 'f'},
        {"input",    required_argument, 0, 'i'},
        {"mode",  required_argument, 0, 'm'},
        {"merge",  required_argument, 0, 'g'},
        {"train",    required_argument, 0, 't'},
        {"resume",  no_argument,       &resume, 1},
        {"samples",    required_argument, 0, 's'},
        {"verbose", no_argument,       &verbose, 1},


        {0, 0, 0, 0}
    };

    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "b:c:d:e:f:i:m:rs:t:vw:", long_options, &option_index)) != -1) {
        switch(opt) {
        case 'l':
            channels = atoi(optarg);
            break;
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
        case 'g':
            mergeFile = optarg;
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
        case 'b':
            blockSize = atoi(optarg);
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

    std::cout << coeffs << " " << blockSize << "  " << dictSize << " "  << trainFile << std::endl;

    Dictionary dict(blockSize, channels, dictSize);

    Coder* coder = 0;
    switch(mode) {
    case 1: coder = new CoderOMP(); break;
    case 2: coder = new CoderLasso(); break;
    }

    coder->eps = eps;
    coder->coeffs = coeffs;



    if(trainFile != "") {
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
            samples.loadImage(nameStr, blockSize, channels, winSize);

//            if(!resume && counter==1){ dict.initFromData(samples); std::cout << "size" << dict.getData().rows() << std::endl; dict.debugSaveImage( (dictFile + ".png").c_str() );}

            std::cout << "train set fill complete " << std::endl;
            if(!running) break;
            trainer.train(samples, dict,  0, sampleCount);
            ifs >> nameStr;
        }
        ifs.close();
//      dict.sort();
        dict.debugSaveImage( (dictFile + ".png").c_str() );
        dict.save( dictFile.c_str() );
//        trainer.save((dictFile + ".tmp").c_str() );
    }


    if(mergeFile!="") {
        std::cout << "merging" << std::endl;
        std::ifstream ifs( (mergeFile).c_str() );
        std::string nameStr;
        ifs >> nameStr;
        int counter=0;
        Dictionary dictIn(1,channels,1);
        while( !ifs.eof() ) {
            if(!running) break;
            std::cout << nameStr << " " << counter++ << std::endl;
            dictIn.load(nameStr.c_str() );
//            dictIn.sort();
            dict.merge(dictIn, eps);
            ifs >> nameStr;
        }
        ifs.close();
        dict.sort();
//        for (int i=0; i<dict.meta_->col_.size(); i++)
//            std::cout << dict.meta_->col_[i].var_ << std::endl;
        dict.debugSaveImage( (dictFile + ".png").c_str() );
        dict.save( dictFile.c_str() );
    }

    if(imageFile!="") {
        Samples samples;
        std::string outputFilename = imageFile + ".recon.png";
        dict.load( dictFile.c_str() );
        samples.loadImage(imageFile, blockSize, channels, blockSize);
        samples.saveImage(outputFilename, dict, *coder);

    }

}

