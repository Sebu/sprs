#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.0f), maxError_(0.10f), maxColor_(1.25f), maxOrient_(150.0f),
        gfNumFeatures_(20), gfQualityLvl_(0.001), gfMinDist_(0.1),
        kltWinSize_(6), kltMaxLvls_(1)
{
}
