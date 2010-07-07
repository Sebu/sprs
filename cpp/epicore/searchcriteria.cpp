#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(0.7f), maxError_(0.16f), maxColor_(1.5f), maxOrient_(.008f),
        gfNumFeatures_(6), gfQualityLvl_(0.01), gfMinDist_(1.0),
        kltWinSize_(15), kltMaxLvls_(1)
{
}
