#include "searchcriteria.h"

SearchCriteria::SearchCriteria() :
        alpha_(1.3f), maxError_(0.16f), maxColor_(1.9f), maxOrient_(.4f),
        gfNumFeatures_(6), gfQualityLvl_(0.1), gfMinDist_(0.5),
        kltWinSize_(4), kltMaxLvls_(1)
{
}
