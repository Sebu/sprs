#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/seedmap.h>

#include <signal.h>
#include <QDir>
#include <QTime>
#include <ctime>

SeedMap* seedmap;

void terminate (int param)
{
    seedmap->termCalculate_ = 1;
}

int main(int argc, char *argv[])
{

    void (*prev_fn)(int);

    prev_fn = signal(SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal(SIGINT,SIG_IGN);
    prev_fn = signal (SIGTERM,terminate);
    if (prev_fn==SIG_IGN) signal(SIGTERM,SIG_IGN);

    // command line parsing
    float error=0.16;
    bool verbose= false;
    bool atlasSearch=0;
    int  s = 12;
    int winsize = 6;
    std::string fullName;

    int opt;
    while ((opt = getopt(argc, argv, "ab:e:i:vw:")) != -1) {
        switch(opt) {
        case 'a':
            atlasSearch = true;
            break;
        case 'b':
            s = atoi(optarg);
            break;
        case 'e':
            error = atof(optarg);
            break;
        case 'i':
            fullName = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'w':
            winsize = atoi(optarg);
            break;
        case '?':
        default: /* '?' */
            std::cerr << "Usage: " << argv[0] << " [-i input_file] [-v] [-e error] [-atlasSearch] [-b blocksize]" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    //std::cin >> fullName;

    size_t found=fullName.find_last_of("/\\");
    std::string pathName = fullName.substr(0,found);
    std::string fileName = fullName.substr(found+1);

    if(verbose)
    std::cout << fileName << " " << pathName << " " << std::endl;

    // original
    cv::Mat image = cv::imread(fullName);
    seedmap = new SeedMap(s, true);
    seedmap->crit_.maxError_ = error;
    seedmap->verbose_ = verbose;
    seedmap->setImage(image);
    //seedmap->deserialize(fullName);

    // for in qdir
    QString epiPath = QString(pathName.c_str()) + "/epitomes/";
    QDir dir(epiPath);


    seedmap->searchInOriginal_ = false;

    cv::Mat base;
    if(atlasSearch) {
        if(verbose) std::cout << "testing epitomes" << std::endl;

        foreach( QString name, dir.entryList(QStringList("*.epi.png")) ) {
            std::string nameStr = (epiPath + name).toStdString();

            if(verbose) std::cout << nameStr << std::endl;
            // size_t found=nameStr.find_last_of(".");
            base = cv::imread(nameStr);
            seedmap->setReconSource(base,1);
            seedmap->matchAll();
            if(seedmap->termCalculate_) break;
        }
    }

    // search in original image
    if(verbose)
    std::cout << "testing original" << std::endl;
    if(!seedmap->termCalculate_ || !seedmap->done_) {
        base = cv::imread(fullName);
        seedmap->searchInOriginal_ = true;
        seedmap->setReconSource(base,3);
        seedmap->matchAll();
        if(!seedmap->termCalculate_) {

            time_t seconds;
            seconds = time(0);
            QString tString( QTime::currentTime().toString("hhmmsszzz") );
            std::string epiName = (epiPath+tString+fileName.c_str()).toStdString();

            if(verbose)
                std::cout << epiName << std::endl;
            seedmap->generateCharts();
            seedmap->image_.saveTexture(epiName);
        }
    }

    // save matches when interrupted
    //if(seedmap->termCalculate_)
      //  seedmap->serialize(fullName);

    // finished? save image
    std::cout << fullName << std::endl;
    if(seedmap->done_)
        seedmap->image_.save(fullName);

    // save a preview of reconstruction
    if(seedmap->done_)
        std::cout << "done" << std::endl;

    seedmap->saveReconstruction(fullName);


}
