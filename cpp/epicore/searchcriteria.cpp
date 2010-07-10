#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(0.8f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(.01f),
        gfNumFeatures_(10), gfQualityLvl_(0.01), gfMinDist_(1.0),
        kltWinSize_(15), kltMaxLvls_(1)
{
}
