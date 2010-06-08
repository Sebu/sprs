#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.35f), maxError_(0.16f), maxColor_(1.25f), maxOrient_(1000.0f),
        gfNumFeatures_(6), gfQualityLvl_(0.1), gfMinDist_(0.5),
        kltWinSize_(8), kltMaxLvls_(1)
{
}
