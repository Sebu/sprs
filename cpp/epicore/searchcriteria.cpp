#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(0.7f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(.008f),
        gfNumFeatures_(20), gfQualityLvl_(0.001), gfMinDist_(0.1),
        kltWinSize_(15), kltMaxLvls_(1)
{
}
