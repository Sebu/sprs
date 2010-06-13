#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.5f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(.3f),
        gfNumFeatures_(6), gfQualityLvl_(0.1), gfMinDist_(1.0),
        kltWinSize_(4), kltMaxLvls_(1)
{
}
