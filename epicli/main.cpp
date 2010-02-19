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
    if(argc<2) return -1;

    void (*prev_fn)(int);
    prev_fn = signal (SIGINT,terminate);
    if (prev_fn==SIG_IGN) signal (SIGINT,SIG_IGN);
    
    std::string fileName(argv[1]);
    
    cv::Mat image = cv::imread( fileName );
    seedmap = new SeedMap( image, image, 16, true);
    seedmap->deserialize(fileName);
    seedmap->maxError_ = atof(argv[2]);
    std::cout << seedmap->maxError_ << std::endl;

    seedmap->matchAll();
    
    seedmap->saveReconstruction((fileName + ".recon.jpg").c_str());
    seedmap->serialize(fileName);
    
}
