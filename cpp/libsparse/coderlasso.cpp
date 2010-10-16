

#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

CoderLasso::CoderLasso()
{
}

vigra::Matrix<double> CoderLasso::code(vigra::Matrix<double>& s, Dictionary& D)
{
    return lasso(s, D.getData());
}
