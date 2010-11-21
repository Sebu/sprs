

#include <algorithm>
#include <eigen2/Eigen/LU>
#include "coderlasso.h"
#include "dictionary.h"
#include "vigra_ext.h"

CoderLasso::CoderLasso()
{
}

Eigen::SparseMatrix<float> CoderLasso::encode(MatrixXf& yM, Dictionary& D) // s D
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

    int L = yM.cols();

    // nvars = min(n-1,p);
    int nvars = std::min(n-1,p);

    int maxk = 8*nvars; // Maximum number of iterations

    MatrixXf Gram = X.transpose()*X;

    MatrixXf beta[L];
    int k[L];
    float stop = -10;

#pragma omp parallel for
for (int signum=0; signum<L; ++signum) {

    MatrixXf y = yM.col(signum);

    //y.normalize();


    if (stop == 0)
      beta[signum] = MatrixXf::Zero(2*nvars, p);
    else if (stop < 0)
      beta[signum] = MatrixXf::Zero(2*round(-stop), p);
    else
      beta[signum] = MatrixXf::Zero(100, p);

    MatrixXf mu = MatrixXf::Zero(n, 1); // current "position" as LARS travels towards lsq solution
    // I = 1:p; % inactive set
    VectorXf I = VectorXf::Ones(p);
    // A = [] ; % active set
    VectorXf A = VectorXf::Zero(p);

    bool lassocond = false; // LASSO condition boolean
    bool stopcond = false; // Early stopping condition boolean
    int vars = 0; // Current number of variables
    k[signum] = 0;  // Iteration count

    // LARS main loop
    // omp it :)
    while ((vars < nvars) && (!stopcond) && (k[signum] < maxk)) {
      k[signum]++;

      VectorXf c = X.transpose()*(y - mu);

      //  [C j] = max(c(I).cwise().abs());
      int j = 0;
      (c.cwise()*I).cwise().abs().maxCoeff(&j);
      float C = abs(c(j));


      if (!lassocond) { // if a variable has been dropped, do one iteration with this configuration (don't add new one right away)
        A(j) = 1.0;
        I(j) = 0.0;
        vars++;
      }

       // s = sign(c(A)); // get the signs of the correlations
       VectorXf s = VectorXf::Zero(vars);
       VectorXf tmp = c.cwise()*A;

       int ii=0;
       for(int i=0; i<tmp.rows(); i++) {
           if(A(i)==1.0) {
               if(tmp(i)>0.0) s(ii) = 1.0;
               else if (tmp(i)<0.0) s(ii) = -1.0;
               else s(ii) = 0.0;
               ii++;
           }
       }


      MatrixXf Gsub(vars,vars);

      ii=0;
      int jj;
      for(int i=0; i<A.size(); i++) {
          if(A(i)==1.0) {
              jj=0;
              for(int k=0; k<A.size(); k++) {
                  if(A(k)==1.0) {
//                    std::cout << "schreib" << ii << " " << jj << "\n" << Gsub << std::endl;
                    Gsub(ii,jj) = Gram(i,k);
                    jj++;
                  }
              }
              ii++;
          }
      }

      VectorXf GA1 = ( Gsub.cwise()*(s*s.transpose()) ).inverse() * VectorXf::Ones(vars);
      float AA = 1.0/(float)(float)GA1.sum(); //sqrt
      MatrixXf w = (AA*GA1).cwise()*s; // weights applied to each active variable to get equiangular direction
//      std::cout << "AA  " << AA << std::endl; //(tmp2.cwise()*(S*S.transpose())) << std::endl;

      MatrixXf Xsub(n,vars);
      ii=0;
      for(int i=0; i<A.size(); i++) {
          if(A(i)==1.0) {
            Xsub.col(ii) = X.col(i);
            ii++;
          }
      }

      MatrixXf u = Xsub*w; // equiangular direction (unit vector)

      float gamma = 0.2;
      if (vars == nvars) { // if all variables active, go all the way to the lsq solution
        gamma = C/AA;
      } else {
        MatrixXf a = Xsub.transpose()*u; // correlation between each variable and eqiangular vector
////        temp = [(C - c(I))./(AA - a(I)); (C + c(I))./(AA + a(I))];
////        gamma = min([temp(temp > 0); C/AA]);
      }

      VectorXf betasub(vars);
      ii=0;
      for(int i=0; i<beta[signum].cols(); i++) {
          if(A(i)==1.0) {
            betasub(ii) = beta[signum](k[signum],i);
            ii++;
          }
      }

      // LASSO modification
      lassocond = false;
      VectorXf temp = -1.0*(betasub.cwise()/w);
      float gamma_tilde = gamma;
////      [gamma_tilde] = min([temp(temp > 0) gamma]);
////      j = find(temp == gamma_tilde);
      if (gamma_tilde < gamma) {
        gamma = gamma_tilde;
        lassocond = true;
      }


      mu += gamma*u;

// TODO: RESIZE
//      if (beta.cols() < k+1) {
//        beta = [beta; zeros(size(beta,1), p)];
//      }

      ii=0;
      for(int i=0; i<beta[signum].cols(); i++) {
          if(A(i)==1.0) {
            beta[signum](k[signum]+1,i) = beta[signum](k[signum],i) + gamma*w(ii);
            ii++;
          }
      }

//      // Early stopping at specified bound on L1 norm of beta
//      if (stop > 0) {
//        float t2 = beta.row(k+1).cwise().abs().sum();
//        if (t2 >= stop) {
//          float t1 = beta.row(k).cwise().abs().sum();
//          float s = (stop - t1)/(t2 - t1); // interpolation factor 0 < s < 1
//          beta.row(k+1) = beta.row(k) + s*(beta.row(k+1) - beta.row(k));
//          stopcond = true;
//        }
//      }

////      // If LASSO condition satisfied, drop variable from active set
      if (lassocond) {
        I(j) = 1.0;
        A(j) = 0.0;
        vars--;
      }

      // Early stopping at specified number of variables
      if (stop < 0)
        stopcond = (vars >= -stop);


    } // while
////    // trim beta
////    if size(beta,1) > k+1
////      beta(k+2:end, :) = [];
////    end

}
    Eigen::SparseMatrix<float> Gamma(X.cols(),L);
    Gamma.startFill();
    for(int signum=0; signum<L; ++signum){
        for (int i=0; i<beta[signum].cols(); i++) {
            if(beta[signum](k[signum],i)!=0.0) {
                Gamma.fillrand(i,signum) = beta[signum](k[signum],i);
            }
        }
    }
    Gamma.endFill();

//    if (k == maxk)
//      std::cout << "LARS warning: Forced exit. Maximum number of iteration reached." << std::endl;

    return Gamma;
}
