

#include <algorithm>
#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

CoderLasso::CoderLasso()
{
}

Eigen::SparseMatrix<float> CoderLasso::encode(MatrixXf& y, Dictionary& D) // s D
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

    MatrixXf X = D.getData();
    // LARS variable setup
    // [n p] = size(X);
    int n = X.rows();
    int p = X.cols();

    // nvars = min(n-1,p);
    int nvars = std::min(n-1,p);

    int maxk = 8*nvars; // Maximum number of iterations

    MatrixXf beta;
    int stop = 10;
    if (stop == 0)
      beta = MatrixXf::Zero(2*nvars, p);
    else if (stop < 0)
      beta = MatrixXf::Zero(2*round(-stop), p);
    else
      beta = MatrixXf::Zero(100, p);

    MatrixXf mu = MatrixXf::Zero(n, 1); // current "position" as LARS travels towards lsq solution
    // I = 1:p; % inactive set
    // A = [] ; % active set


    MatrixXf Gram = X.transpose()*X;

    bool lassocond = false; // LASSO condition boolean
    bool stopcond = false; // Early stopping condition boolean
    int k = 0;  // Iteration count
    int vars = 0; // Current number of variables

    // LARS main loop
    // omp it :)
//    while (vars < nvars && !stopcond && k < maxk) {
//      k = k + 1;
//      MatrixXf c = X.transpose()*(y - mu);
////      [C j] = max(abs(c(I)));
////      j = I(j);

//      if (!lassocond) { // if a variable has been dropped, do one iteration with this configuration (don't add new one right away)
////        A = [A j];
////        I(I == j) = [];
//        vars = vars + 1;
//      }

////      s = sign(c(A)); // get the signs of the correlations

//      MatrixXf S = s*MatrixXf::Ones(1,vars);
//      MatrixXf GA1 = inv(Gram(A,A).cwise()*S.transpose().cwise()*S)*MatrixXf::Ones(vars,1);
//      MatrixXf AA = 1/sqrt(sum(GA1));
//      MatrixXf w = AA*GA1.cwise()*s; // weights applied to each active variable to get equiangular direction

////      u = X(:,A)*w; // equiangular direction (unit vector)

//      if (vars == nvars) // if all variables active, go all the way to the lsq solution
//        gamma = C/AA;
//      else {
//        a = X.transpose()*u; // correlation between each variable and eqiangular vector
////        temp = [(C - c(I))./(AA - a(I)); (C + c(I))./(AA + a(I))];
////        gamma = min([temp(temp > 0); C/AA]);
//      }

//      // LASSO modification
//      lassocond = 0;
//      temp = -beta(k,A).cwise()/w.transpose();
////      [gamma_tilde] = min([temp(temp > 0) gamma]);
////      j = find(temp == gamma_tilde);
//      if (gamma_tilde < gamma,) {
//        gamma = gamma_tilde;
//        lassocond = 1;
//      }


//      mu = mu + gamma*u;
//      if (beta.cols() < k+1) {
////        beta = [beta; zeros(size(beta,1), p)];
//      }
////      beta(k+1,A) = beta(k,A) + gamma*w.transpose();

//      // Early stopping at specified bound on L1 norm of beta
//      if (stop > 0) {
////        t2 = sum(abs(beta(k+1,:)));
//        if (t2 >= stop) {
////          t1 = sum(abs(beta(k,:)));
//          s = (stop - t1)/(t2 - t1); // interpolation factor 0 < s < 1
////          beta(k+1,:) = beta(k,:) + s*(beta(k+1,:) - beta(k,:));
//          stopcond = 1;
//        }
//      }

////      // If LASSO condition satisfied, drop variable from active set
//      if (lassocond) {
////        I = [I A(j)];
////        A(j) = [];
//        vars = vars - 1;
//      }

////      // Early stopping at specified number of variables
////      if (stop < 0)
////        stopcond = vars >= -stop;

//    } // while

////    // trim beta
////    if size(beta,1) > k+1
////      beta(k+2:end, :) = [];
////    end

//    if (k == maxk)
//      std::cout << "LARS warning: Forced exit. Maximum number of iteration reached." << std::endl;
}
