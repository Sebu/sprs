#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/seedmap.h>

#include <signal.h>

bool termCalculate=0;
SeedMap* seedmap;

void terminate (int param)
{
   termCalculate = 1;
   seedmap->termCalculate = 1;
}

int main(int argc, char *argv[])
{
    if(argc<2) return -1;

    void (*prev_fn)(int);
    prev_fn = signal (SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal (SIGINT,SIG_IGN);
    
    std::string fileName(argv[1]);
    
    cv::Mat image = cv::imread( fileName );
    seedmap = new SeedMap( image, 16);
    seedmap->loadMatches(fileName);
    seedmap->maxError = atof(argv[2]);
    std::cout << seedmap->maxError << std::endl;

    while(seedmap->matchNext() && !termCalculate) {}
    
    seedmap->saveReconstruction((fileName + ".recon.jpg").c_str());
    seedmap->saveMatches(fileName);
    
}
