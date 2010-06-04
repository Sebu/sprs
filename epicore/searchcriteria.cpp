#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.5f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(250.0f),
        gfNumFeatures_(6), gfQualityLvl_(0.04), gfMinDist_(1.0),
        kltWinSize_(3), kltMaxLvls_(1)
{
}
