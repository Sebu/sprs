#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.0f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(.5f),
        gfNumFeatures_(10), gfQualityLvl_(0.02), gfMinDist_(1.0),
        kltWinSize_(10), kltMaxLvls_(1)
{
}
