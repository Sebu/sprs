

#include <algorithm>
#include <eigen2/Eigen/LU>
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
    int stop = -2;
    if (stop == 0)
      beta = MatrixXf::Zero(2*nvars, p);
    else if (stop < 0)
      beta = MatrixXf::Zero(2*round(-stop), p);
    else
      beta = MatrixXf::Zero(100, p);

    MatrixXf mu = MatrixXf::Zero(n, 1); // current "position" as LARS travels towards lsq solution
    // I = 1:p; % inactive set
    VectorXf I = VectorXf::Ones(p);
    // A = [] ; % active set
    VectorXf A = VectorXf::Zero(p);


    MatrixXf Gram = X.transpose()*X;

    bool lassocond = false; // LASSO condition boolean
    bool stopcond = false; // Early stopping condition boolean
    int k = 0;  // Iteration count
    int vars = 0; // Current number of variables

    // LARS main loop
    // omp it :)
    while ((vars < nvars) && (!stopcond) && (k < maxk)) {
      k++;

      VectorXf c = X.transpose()*(y - mu);

      //  [C j] = max(c(I).cwise().abs());
      int j = 0;
      (c.cwise()*I).cwise().abs().maxCoeff(&j);
      float C = abs(c(j));

      //j = I[j]; // j = I(j);

      if (!lassocond) { // if a variable has been dropped, do one iteration with this configuration (don't add new one right away)
        //  A = [A j];
        A(j) = 1.0;

        // I(I == j) = [];
        I(j) = 0.0;

        vars++;
      }

       // s = sign(c(A)); // get the signs of the correlations
       VectorXf s = VectorXf::Zero(c.rows());
       VectorXf tmp = c.cwise()*A;
       for(int i=0; i<tmp.rows(); i++) {
           if(tmp(i)>0.0) s(i) = 1;
           else if (tmp(i)<0.0) s(i) = -1;
           else s(i) = 0;
       }



      MatrixXf S = s*MatrixXf::Ones(1, c.rows()); //vars);
      MatrixXf tmp2 = Gram.cwise()*(A*A.transpose());
      VectorXf GA1 = ((tmp2.cwise()*S.transpose()).cwise()*S).inverse()*MatrixXf::Ones(c.rows(),1);
      float AA = 1.0/(float)sqrt((double)GA1.sum());
      MatrixXf w = (AA*GA1).cwise()*s; // weights applied to each active variable to get equiangular direction
      std::cout << (tmp2.cwise()*S.transpose()).cwise()*S << std::endl;

      // u = X(:,A)*w; // equiangular direction (unit vector)
//      std::cout << w.rows() << " " << w.cols() << " " << GA1.cols() << " " <<  GA1.cols() << std::endl;
      MatrixXf u = X.cwise()*(VectorXf::Ones(X.rows())*A.transpose())*w; // equiangular direction (unit vector)
//      std::cout << "U:" << u << std::endl;
      float gamma = 1.0;
      if (vars == nvars) { // if all variables active, go all the way to the lsq solution
        gamma = C/AA;
      } else {
//        a = X.transpose()*u; // correlation between each variable and eqiangular vector
////        temp = [(C - c(I))./(AA - a(I)); (C + c(I))./(AA + a(I))];
////        gamma = min([temp(temp > 0); C/AA]);
      }

//      // LASSO modification
      lassocond = false;
      VectorXf temp = ( -1.0*(beta.row(k).cwise()*A.transpose()) ).cwise()/w.transpose();
      float gamma_tilde = gamma;
////      [gamma_tilde] = min([temp(temp > 0) gamma]);
////      j = find(temp == gamma_tilde);
      if (gamma_tilde < gamma) {
        gamma = gamma_tilde;
        lassocond = true;
      }


      mu += gamma*u;
//      if (beta.cols() < k+1) {
//        beta = [beta; zeros(size(beta,1), p)];
//      }
      beta.row(k+1) = beta.row(k) + (gamma*w.transpose());
//      std::cout << w.transpose() << std::endl;

//      // Early stopping at specified bound on L1 norm of beta
      if (stop > 0) {
        float t2 = beta.row(k+1).cwise().abs().sum();
        if (t2 >= stop) {
          float t1 = beta.row(k).cwise().abs().sum();
          float s = (stop - t1)/(t2 - t1); // interpolation factor 0 < s < 1
          beta.row(k+1) = beta.row(k) + s*(beta.row(k+1) - beta.row(k));
          stopcond = true;
        }
      }

////      // If LASSO condition satisfied, drop variable from active set
      if (lassocond) {
        // I = [I A(j)];
        I(j) = 1.0;
        // A(j) = [];
        A(j) = 0.0;
        vars--; //= vars - 1;
      }

      // Early stopping at specified number of variables
      if (stop < 0)
        stopcond = (vars >= -stop);


    } // while

////    // trim beta
////    if size(beta,1) > k+1
////      beta(k+2:end, :) = [];
////    end
    Eigen::SparseMatrix<float> Gamma(X.cols(),1);
    Gamma.startFill();
    for (int i=0; i<beta.cols(); i++) {
        if(beta(k,i)!=0.0) {
//            std::cout << beta(k,i) << std::endl;
            Gamma.fillrand(i,0) = beta(k,i);
        }
    }
    Gamma.endFill();

    if (k == maxk)
      std::cout << "LARS warning: Forced exit. Maximum number of iteration reached." << std::endl;

    return Gamma;
}
