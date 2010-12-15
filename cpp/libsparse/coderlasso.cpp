

#include <algorithm>
#include <float.h>
#include <eigen2/Eigen/LU>
#include <eigen2/Eigen/Array>
#include <eigen2/Eigen/Cholesky>
#include <list>
#include <algorithm>
#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

CoderLasso::CoderLasso()
{
}

Eigen::SparseMatrix<double> CoderLasso::encode(MatrixXd& yM, Dictionary& D) // s D
{
//    return lasso(s, D.getData());

//    %    BETA = LARS(X, Y) performs least angle regression on the variables in
//    %    X to approximate the response Y. Variables X are assumed to be
//    %    normalized (zero mean, unit length), the response Y is assumed to be
//    %    centered.
//    %    BETA = LARS(X, Y, METHOD, STOP) with nonzero STOP will perform least
//    %    angle or lasso regression with early stopping. If STOP is negative,
//    %    STOP is an integer that determines the desired number of variables. If
//    %    STOP is positive, it corresponds to an upper bound on the L1-norm of
//    %    the BETA coefficients.
//    %    Returns BETA where each row contains the predictor coefficients of
//    %    one iteration. A suitable row is chosen using e.g. cross-validation,
//    %    possibly including interpolation to achieve sub-iteration accuracy.

    // Input checking
    // Set default values.



    MatrixXd X = D.getData();
    // LARS variable setup
    // [n p] = size(X);
    int n = X.rows();
    int p = X.cols();

    int L = yM.cols();

    int nvars = std::min(n-1,p);

    int maxk = 8*nvars; // Maximum number of iterations

    MatrixXd Gram = X.transpose()*X;

    MatrixXd beta[L];
    int k[L];
    // A = [] ; % active set
    std::vector<int> A[L];
    double stop = -this->coeffs;

    std::cout << "precalc done" << std::endl;

#pragma omp parallel for
for (int signum=0; signum<L; ++signum) {

    MatrixXd y = yM.col(signum);


    beta[signum] = MatrixXd::Zero(1, p);


    bool lassocond = false; // LASSO condition boolean
    bool stopcond = false; // Early stopping condition boolean
    int vars = 0; // Current number of variables
    k[signum] = 0;  // Iteration count

    if(y.isZero()) {
       // std::cout << "lucky trivial bastard" << std::endl;
        continue;
    }

    MatrixXd mu = MatrixXd::Zero(n, 1); // current "position" as LARS travels towards lsq solution

    // I = 1:p; % inactive set
    std::vector<int> I;
    for(int i=0; i<p; i++)
        I.push_back(i);


    // LARS main loop
    int j = 0;

    while ((vars < nvars) && (!stopcond) && (k[signum] < maxk)) {
      k[signum]++;

      // find highest corelation
      VectorXd c = X.transpose()*(y - mu);
      VectorXd csub(p-vars);
      for(int i=0; i<I.size(); i++)
          csub(i) = c( I[i] );
      csub.cwise().abs().maxCoeff(&j);
      double C = std::abs(csub(j));
      j = I[j];


      if (!lassocond) { // if a variable has been dropped, do one iteration with this configuration (don't add new one right away)
        A[signum].push_back(j);
        std::vector<int>::iterator it;
        it = std::find(I.begin(),I.end(),j);
        I.erase(it);
        vars++;
      }

       // s = sign(c(A)); // get the signs of the correlations
       VectorXd s = VectorXd::Zero(A[signum].size());

       VectorXd csub2(A[signum].size());
       for(int i=0; i<A[signum].size(); i++)
           csub2(i) = c( A[signum][i] );

       for(int i=0; i<csub2.rows(); i++) {
         if(csub2(i)>0.0) s(i) = 1.0;
         else if (csub2(i)<0.0) s(i) = -1.0;
         else s(i) = 0.0;
       }


      MatrixXd Gsub(vars,vars);
      for(int i=0; i<A[signum].size(); i++) {
         for(int m=0; m<A[signum].size(); m++) {
             Gsub(i,m) = Gram(A[signum][i],A[signum][m]);
          }
      }

      VectorXd GA1 = VectorXd::Ones(vars);
      ( Gsub.cwise()*(s*s.transpose()) ).llt().solveInPlace(GA1);
      //VectorXd GA1 = ( Gsub.cwise()*(s*s.transpose()) ).inverse() * VectorXd::Ones(vars);
      double GA1sum = sqrt(GA1.sum());
      double AA = 1.0/GA1sum;
      if(isinf(GA1sum) || GA1sum!=GA1sum || GA1sum == 0.0) {
          std::cout << Gsub << "\n" << GA1sum << "\n" << y << "\n" << mu << std::endl;
          ei_assert(0 && "wurst");
      }
      MatrixXd w = (AA*GA1).cwise()*s; // weights applied to each active variable to get equiangular direction

      MatrixXd Xsub = subselect(X,A[signum]);


      MatrixXd u = Xsub*w; // equiangular direction (unit vector)

      double gamma = C/AA; // if all variables active, go all the way to the lsq solution

      double erg3 = DBL_MAX;
      if (vars != nvars) {
        VectorXd a = X.transpose()*u;
        for(int i=0; i<I.size(); i++) {
          double erg1 = (C-c(I[i]))/(AA-a(I[i]));
          double erg2 = (C+c(I[i]))/(AA+a(I[i]));
          if(erg1>0.0) erg3=std::min(erg1,erg3);
          if(erg2>0.0) erg3=std::min(erg2,erg3);
        }
        gamma = std::min(erg3,gamma);

      }
      VectorXd betasub(vars);
      for(int i=0; i<A[signum].size(); i++) {
        betasub(i) = beta[signum](0,A[signum][i]);
      }

      // LASSO modification
      lassocond = false;
      VectorXd temp = -1.0*(betasub.cwise()/w);
      double erg = DBL_MAX;
      for(int i=0; i<temp.size(); i++) {
          if(temp(i)>0.0) {
              if(temp(i)<erg) erg=temp(i);
          }
      }
      if (erg < gamma) {
        gamma = erg;
        for(int i=0; i<temp.size(); i++)  {
          if(temp(i)==gamma) j=i;
        }
        lassocond = true;
      }

      //
      mu += gamma*u;

      MatrixXd beta_next = MatrixXd::Zero(1, p);

      for(int i=0; i<A[signum].size(); i++) {
            beta_next(0,A[signum][i]) = beta[signum](0,A[signum][i]) + gamma*w(i);
      }

      beta[signum]=beta_next;


//      // Early stopping at specified bound on L1 norm of beta
//      if (stop > 0) {
//        double t2 = beta.row(k+1).cwise().abs().sum();
//        if (t2 >= stop) {
//          double t1 = beta.row(k).cwise().abs().sum();
//          double s = (stop - t1)/(t2 - t1); // interpolation factor 0 < s < 1
//          beta.row(k+1) = beta.row(k) + s*(beta.row(k+1) - beta.row(k));
//          stopcond = true;
//        }
//      }

      // If LASSO condition satisfied, drop variable from active set
      if (lassocond) {
        I.push_back(A[signum][j]);
        std::vector<int>::iterator it;
        it = A[signum].begin();
        std::advance(it,j);
        A[signum].erase(it);
        vars--;
      }

      // Early stopping at specified number of variables
      if (stop < 0)
        stopcond = (vars >= -stop);


    } // while
}
    std::cout << "sparse code done" << std::endl;

    Eigen::SparseMatrix<double> Gamma(X.cols(),L);
    Gamma.startFill();
    for(int signum=0; signum<L; ++signum){
        for(int i=0; i<A[signum].size(); i++) {
            Gamma.fillrand(A[signum][i],signum) = beta[signum](0,A[signum][i]);
        }
    }
    Gamma.endFill();

    std::cout << "copy done" << std::endl;

//    if (k == maxk)
//      std::cout << "LARS warning: Forced exit. Maximum number of iteration reached." << std::endl;

    return Gamma;
}
