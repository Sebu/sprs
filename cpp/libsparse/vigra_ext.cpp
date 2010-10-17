#include "vigra_ext.h"

int maxabs(Matrix<double>& c)
{
  int maxid=1, k;
  int m = c.rowCount();
  double absval, maxval = 0;

  for (k=1; k<=m; ++k) {
    absval = fabs(c(k,1));
    if (absval > maxval) {
      maxval = absval;
      maxid = k;
    }
  }
  return maxid;
}


void vec_assign(Matrix<double>& y, Matrix<double>& x, Matrix<int>& ind, int k, int start)
{
  int i;
  for (i=0; i<k; ++i)
    y(i,1) = x(ind(i,1), start+1);
}


Matrix<double> dense_vector(ArrayVector<int>  active_set, Matrix<double>  sparse_vector, int size) {

    Matrix<double> dense_vector(size,1);
    dense_vector.init(0.0);
    for (unsigned int i = 0; i < active_set.size(); i++)
        dense_vector(active_set[i],0) = sparse_vector(i,0);

    return dense_vector;
}

Matrix<double> lasso(Matrix<double>& x, Matrix<double>& D) {

    int bestIndex = 0;
    double bestError = FLT_MAX;
    ArrayVector<ArrayVector<int> > active_sets;
    ArrayVector<Matrix<double> > solutions;


    LeastAngleRegressionOptions opts;
    opts.lasso();
    opts.maxSolutionCount(20);
    // run leastAngleRegression() in  LASSO mode
    int numSolutions = leastAngleRegression(D, x, active_sets, solutions, opts);


    //std::cout << bestIndex << std::endl;

//    for (MultiArrayIndex k = 0; k < numSolutions; ++k) {
//        Matrix<double> dense_solution = dense_vector(active_sets[k], solutions[k], D.columnCount());
//        double lsq = (mmul(D,dense_solution)-x).squaredNorm();
//        //        std::cout <<  k << " " << sum(solutions[k]) <<  " " << solutions[k].size() << std::endl;
//        double error = 0.5*lsq + sum(solutions[k]);
//        if(error<bestError) { bestError=error; bestIndex=k; }
//    }

    bestIndex = numSolutions - 1;
    return dense_vector(active_sets[bestIndex], solutions[bestIndex], D.columnCount());
}
