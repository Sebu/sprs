#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.5f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(250.0f),
        gfNumFeatures_(6), gfQualityLvl_(0.01), gfMinDist_(0.1),
        kltWinSize_(5), kltMaxLvls_(1)
{
}
