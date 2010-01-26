#include <fstream>

#include "match.h"
#include "cv_ext.h"
#include "matrix.h"

Match::Match(Patch* seed)
    : seed(0), colorScale(cv::Scalar::all(1.0f)), error(cv::Scalar::all(0.0f)), seedX(0), seedY(0), seedW(0), seedH(0), scale(0.0)
{
    rotMat = cv::Mat::eye(3,3,CV_64FC1);
    warpMat = cv::Mat::eye(3,3,CV_64FC1);
    scaleMat = cv::Mat::eye(3,3,CV_64FC1);
    setSeed(seed);

}

void Match::setSeed(Patch* seed) {
    if (!seed) return;
    seedX = seed->x_;
    seedY = seed->y_;
    seedW = seed->w_;
    seedH = seed->h_;
    scale = seed->scale;
    scaleMat /= scale;

    sourceImage = seed->sourceImage_;
}

Polygon Match::getMatchbox() {
    double points[4][2] = { {seedX      , seedY},
                            {seedX+seedW, seedY},
                            {seedX+seedW, seedY+ seedH},
                            {seedX      , seedY+ seedH}
    };
    Polygon box;

    cv::Mat transform = warpMat * (rotMat*scaleMat);
    cv::Mat selection(transform, cv::Rect(0,0,3,2));
    cv::Mat inverted;
    invertAffineTransform(selection, inverted);

    for(int i=0; i<4; i++ ) {
        cv::Mat p = (cv::Mat_<double>(3,1) << points[i][0], points[i][1], 1);

       cv::Mat a =  inverted * p;

        Vector2f point;
        point.m_v[0] = a.at<double>(0,0);
        point.m_v[1] = a.at<double>(0,1);
        box.verts.push_back(point);

    }
    return box;
}

void Match::deserialize(std::ifstream& ifs) {

    ifs >> seedX >> seedY >> seedW >> seedH >> scale;
    scaleMat /= scale;
    ifs.ignore(8192, '\n');
    // omg O_o two times the same code
    for (int i=0; i<2; i++)
        for(int j=0; j<rotMat.cols; j++)
            ifs >> rotMat.at<double>(i,j);
    ifs.ignore(8192, '\n');
    for (int i=0; i<2; i++)
        for(int j=0; j<warpMat.cols; j++)
            ifs >> warpMat.at<double>(i,j);
    ifs.ignore(8192, '\n');
    ifs >> colorScale[0] >> colorScale[1] >>  colorScale[2];
    ifs.ignore(8192, '\n');
    ifs >> error[0] >> error[1] >> error[2];
    ifs.ignore(8192, '\n');
}

void Match::serialize(std::ofstream& ofs) {
    ofs << seedX << " " << seedY << " " << seedW << " " << seedH << " " << scale << std::endl;

    // omg O_o two times the same code
    for (int i=0; i<2; i++)
        for(int j=0; j<rotMat.cols; j++)
            ofs << rotMat.at<double>(i,j) << " ";
    ofs << "rotation" << std::endl;
    for (int i=0; i<2; i++)
        for(int j=0; j<warpMat.cols; j++)
            ofs << warpMat.at<double>(i,j) << " ";
    ofs << "warp" << std::endl;
    ofs << colorScale[0] << " " << colorScale[1] << " " << colorScale[2] << std::endl;
    ofs << error[0] << " " <<  error[1] << " " << error[2] << std::endl;
}


cv::Mat Match::rotate() {

    cv::Mat transform = rotMat * scaleMat;
    cv::Mat rotated;
    cv::Mat selection(transform, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceImage, rotated, selection, sourceImage.size());

    return rotated;

}

cv::Mat Match::warp() {

    cv::Mat transform = warpMat * (rotMat * scaleMat);
    cv::Mat warped;
    cv::Mat selection(transform, cv::Rect(0,0,3,2));
    cv::warpAffine(sourceImage, warped, selection, sourceImage.size());

    return warped;

}


cv::Mat Match::reconstruct() {

    cv::Mat warped = warp();

    // extract patch
    cv::Mat result( seedW, seedH, sourceImage.type() );
    copyBlock(warped, result, cvRect(seedX, seedY, seedW, seedH), cvRect(0, 0, seedW, seedH));

    // color scale
    std::vector<cv::Mat> planes;
    split(result, planes);
    for(uint i=0; i<planes.size(); i++) {
        planes[i] *= colorScale.val[i];
    }
    merge(planes, result);



    return result;
}


