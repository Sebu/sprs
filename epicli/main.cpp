#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/seedmap.h>

#include <signal.h>
#include<QDir>

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


    std::string fullName;
    std::cin >> fullName;

    size_t found=fullName.find_last_of("/\\");
    std::string pathName = fullName.substr(0,found);
    std::string fileName = fullName.substr(found+1);

    std::cout << fileName << " " << pathName << " " << std::endl;

    // original
    cv::Mat image = cv::imread(fullName);
    seedmap = new SeedMap( image, 16, true);
    seedmap->maxError_ = 0.007; //atof(argv[1]);
    seedmap->deserialize(fullName);

    // for in qdir
    QString epiPath = QString(pathName.c_str()) + "/epitomes/";
    QDir dir(epiPath);


    seedmap->searchInOriginal_ = false;

    std::cout << "testing epitomes" << std::endl;
    foreach( QString name, dir.entryList(QStringList("*.epi.png")) ) {
        std::string nameStr = (epiPath + name).toStdString();
        std::cout << nameStr << std::endl;
//        size_t found=nameStr.find_last_of(".");
        cv::Mat base = cv::imread(nameStr);
        seedmap->setReconSource(base,1);
        seedmap->matchAll();
        if(seedmap->termCalculate_) break;
    }

    std::cout << "testing original" << std::endl;

    // search in original image
    if(!seedmap->termCalculate_ || !seedmap->done_) {
        cv::Mat base = cv::imread(fullName);
        seedmap->searchInOriginal_ = true;
        seedmap->setReconSource(base,3);
        seedmap->matchAll();
        if(!seedmap->termCalculate_) {
            std::string epiName = (epiPath+fileName.c_str()).toStdString();
            std::cout << epiName << std::endl;
            seedmap->generateEpitome();
            seedmap->saveEpitome(epiName);
        }
    }

    // save matches when interrupted
    if(seedmap->termCalculate_)
        seedmap->serialize(fullName);

    // finished? save image
    if(seedmap->done_)
        seedmap->saveCompressedImage(fullName);

    // save a preview of reconstruction
    seedmap->saveReconstruction(fullName);


}
