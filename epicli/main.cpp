//#include <QtCore/QCoreApplication>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <epicore/seedmap.h>

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    std::string fileName(argv[1]);

    cv::Mat image = cv::imread( fileName );
    SeedMap seedmap( image, 16, 16, 4, 4);
    seedmap.maxError = 0.6;

    seedmap.resetMatches();

    while(seedmap.matchNext()) {}

    std::ofstream ofs( (fileName + ".txt").c_str() );
    ofs << "version 1.0" << std::endl;
    ofs << fileName << std::endl;
    seedmap.saveMatches(ofs);
    ofs.close();

    seedmap.saveReconstruction((fileName + ".recon.jpg").c_str());
    //return a.exec();
}
