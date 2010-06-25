#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.4f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(.6f),
        gfNumFeatures_(10), gfQualityLvl_(0.01), gfMinDist_(1.0),
        kltWinSize_(5), kltMaxLvls_(1)
{
}
