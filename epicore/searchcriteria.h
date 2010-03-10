#ifndef SEARCHCRITERIA_H
#define SEARCHCRITERIA_H

class SearchCriteria
{
public:
    float alpha_;
    float maxError_;
    float maxColor_;
    float maxOrient_;
    int winSize_;

    SearchCriteria();
};

#endif // SEARCHCRITERIA_H
