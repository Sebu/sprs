#include "vigra_ext.h"

int maxabs(MatrixXf& c)
{
  int maxid=1, k;
  int m = c.rows();
  double absval, maxval = 0;

  for (k=0; k<m; ++k) {
    absval = fabs(c(k,0));
    if (absval > maxval) {
      maxval = absval;
      maxid = k;
    }
  }
  return maxid;
}


void vec_assign(MatrixXf& y, MatrixXf& x, MatrixXi& ind, int k, int start)
{
  int i;
  for (i=0; i<k; ++i)
    y(i,0) = x(ind(i,0), start);
}


vigra::Matrix<double> dense_vector(vigra::ArrayVector<int>  active_set, vigra::Matrix<double>  sparse_vector, int size) {

    vigra::Matrix<double> dense_vector(size,1);
    dense_vector.init(0.0);
    for (unsigned int i = 0; i < active_set.size(); i++)
        dense_vector(active_set[i],0) = sparse_vector(i,0);

    return dense_vector;
}

vigra::Matrix<double> lasso(vigra::Matrix<double>& x, vigra::Matrix<double>& D) {

    int bestIndex = 0;
    vigra::ArrayVector<vigra::ArrayVector<int> > active_sets;
    vigra::ArrayVector<vigra::Matrix<double> > solutions;


    vigra::linalg::LeastAngleRegressionOptions opts;
    opts.lasso();
    opts.maxSolutionCount(10);
    // run leastAngleRegression() in  LASSO mode
    int numSolutions = vigra::linalg::leastAngleRegression(D, x, active_sets, solutions, opts);


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
