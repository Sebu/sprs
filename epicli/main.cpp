#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/seedmap.h>

#include <signal.h>

bool termCalculate=0;

void terminate (int param)
{
   termCalculate = 1;
}

int main(int argc, char *argv[])
{
    if(argc<2) return -1;

    void (*prev_fn)(int);
    prev_fn = signal (SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal (SIGINT,SIG_IGN);
    
    std::string fileName(argv[1]);
    
    cv::Mat image = cv::imread( fileName );
    SeedMap seedmap( image, 16, 16, 4, 4);
    seedmap.maxError = 0.6;
    seedmap.loadMatches(fileName);


    while(seedmap.matchNext() && !termCalculate) {}
    
    seedmap.saveReconstruction((fileName + ".recon.jpg").c_str());
    seedmap.saveMatches(fileName);
    
}
