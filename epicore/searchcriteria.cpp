#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.2f), maxError_(0.16f), maxColor_(1.25f), maxOrient_(350.0f),
        gfNumFeatures_(3), gfQualityLvl_(0.01), gfMinDist_(0.1),
        kltWinSize_(8), kltMaxLvls_(1)
{
}
