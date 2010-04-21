#ifndef SEARCHCRITERIA_H
#define SEARCHCRITERIA_H

class SearchCriteria
{
public:

    float alpha_;
    float maxError_;
    float maxColor_;
    float maxOrient_;

    // good featues
    int gfNumFeatures_;
    double gfQualityLvl_;
    double gfMinDist_;

    // KLT
    int kltWinSize_;
    int kltMaxLvls_;

    SearchCriteria();
};

#endif // SEARCHCRITERIA_H
