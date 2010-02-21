#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/seedmap.h>

#include <signal.h>

SeedMap* seedmap;

void terminate (int param)
{
   seedmap->termCalculate_ = 1;
}

int main(int argc, char *argv[])
{

    void (*prev_fn)(int);
    prev_fn = signal (SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal (SIGINT,SIG_IGN);
    
    std::string fileName;
    std::cin >> fileName;

    std::cout << fileName << " " << argv[1] << std::endl;

    cv::Mat image = cv::imread( fileName );
    seedmap = new SeedMap( image, image, 16, true);


    seedmap->maxError_ = 0.007; //atof(argv[1]);

    seedmap->deserialize(fileName);
    seedmap->matchAll();
    seedmap->serialize(fileName);

    seedmap->generateEpitomes();

    seedmap->save((fileName + ".map").c_str());

    seedmap->saveReconstruction((fileName + ".recon.png").c_str());

    seedmap->saveEpitome(fileName);

}
