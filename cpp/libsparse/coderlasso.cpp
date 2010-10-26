

#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

CoderLasso::CoderLasso()
{
}

vigra::Matrix<double> CoderLasso::encode(vigra::Matrix<double>& s, Dictionary& D)
{
    return lasso(s, D.getData());
}
