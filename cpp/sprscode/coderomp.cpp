#include "coderomp.h"
#include "vigra_ext.h"
#include "dictionary.h"
#include <math.h>
#include <string.h>

CoderOMP::CoderOMP()
{
}

Eigen::SparseMatrix<double> CoderOMP::encode(MatrixXd& X, Dictionary& D)
{
    int signum;
    MatrixXd DtX, XNorm, G;


    int T = this->coeffs;
    double eps = this->eps;
    bool erroromp = false;
    if(eps>0.0) {
        erroromp = true;
    }

    int m = D.getData().cols();
    int n = X.rows();
    int L = X.cols();

    /* precalculate for speed */

    #pragma omp parallel sections
    {
        #pragma omp section
        DtX = D.getData().transpose()*X;
        #pragma omp section
        if(erroromp)
            XNorm = (X.cwise()*X).colwise().sum();
        #pragma omp section
        G = D.getData().transpose()*D.getData();
    }

//    std::cout << "precalc done" << std::endl;

    Eigen::SparseMatrix<double> GammaVector[L];

    /* current number of columns in Dsub / Gsub / Lchol */
    int allocated_cols = T; //erroromp ? (int)(ceil(sqrt((double)n)/2.0) + 1.01) : T;

//    std::cout  << "prep done" << std::endl;
    /**********************   perform omp for each signal   **********************/
    #pragma omp parallel for
    for (signum=0; signum<L; ++signum) {
        int i, j, pos;
        double eps2, resnorm, delta, deltaprev;

        /*** helper arrays ***/
        VectorXd alpha;             /* contains D'*residual */
        VectorXi ind(n);            /* indices of selected atoms */
        VectorXi selected_atoms(m); /* binary array with 1's for selected atoms */
        VectorXd c(allocated_cols);           /* orthogonal projection result */
        /* Cholesky decomposition of D_I'*D_I */
        MatrixXd Lchol(n,allocated_cols);
        /* temporary vectors for various computations */
        MatrixXd tempvec1(m,1);
        MatrixXd tempvec2(m,1);
        /* matrix containing G(:,ind) - the columns of G corresponding to the selected atoms, in order of selection */
        MatrixXd Gsub(m,allocated_cols);

        tempvec1.setZero(); //init(0.0);
        tempvec2.setZero();
        c.setZero();
        Gsub.setZero();
        Lchol.setZero();
        GammaVector[signum].resize(m,1);

        /* initialize residual norm and deltaprev for error-omp */

        if (erroromp) {
            resnorm = XNorm(0,signum);
            deltaprev = 0;     /* delta tracks the value of gamma'*G*gamma */
            /*** initializations for error omp ***/
            eps2 = eps*eps;        /* compute eps^2 */
            if (T<0 || T>n) {      /* unspecified max atom num - set max atoms to n */
                T = n;
            }
        }
        else {
            /* ignore residual norm stopping criterion */
            eps2 = 0;
            resnorm = 1;
        }

            /* initialize alpha := DtX */
            alpha = DtX.col(signum);

            /* mark all atoms as unselected */
            for (i=0; i<m; ++i) {
                selected_atoms(i) = 0;
            }


        /* main loop */
        i=0;
        while (resnorm>eps2 && i<T) {

            pos = maxabs(alpha);

            /* stop criterion: selected same atom twice, or inner product too small */

            if (selected_atoms(pos,0) || alpha(pos,0)*alpha(pos,0)<1e-14) {
                break;
            }


            /* mark selected atom */

            ind(i) = pos;
            selected_atoms(pos) = 1;


             /* append column to Gsub */
            Gsub.col(i) = G.col(pos);

            /*** Cholesky update ***/
            if (i==0) {
                Lchol(0,0) = 1.0;
            }
            else {

                /* incremental Cholesky decomposition: compute next row of Lchol */


                vec_assign(tempvec1, Gsub, ind, i, i);            /* extract tempvec1 := Gsub(ind,i) */

//              /* compute tempvec2 = Lchol \ tempvec1 */
                tempvec2.block(0,0,i,1) = Lchol.block(0,0, i,i).part<Eigen::LowerTriangular>().solveTriangular(tempvec1.block(0,0, i,1));

                Lchol.block(i,0,1,i) = tempvec2.transpose().block(0,0,1,i);

                /* compute Lchol(i,i) */
                double sum = 1.0-(tempvec2.block(0,0,i,1).transpose()*tempvec2.block(0,0,i,1))(0,0);
                if ( sum <= 1e-14 ) {     /* Lchol(i,i) is zero => selected atoms are dependent */
                    break;
                }
                Lchol(i,i) = sqrt(sum);
            }


            i++;

            /* perform orthogonal projection and compute sparse coefficients */

            vec_assign(tempvec1, DtX, ind, i, signum);           /* extract tempvec1 = DtX(ind) */

//             /* solve LL'c = tempvec1 for c */
            c.block(0,0,i,1) = Lchol.block(0,0, i,i).part<Eigen::LowerTriangular>().solveTriangular(tempvec1.block(0,0, i,1));
            Lchol.block(0,0, i,i).transpose().part<Eigen::UpperTriangular>().solveTriangularInPlace(c.block(0,0,i,1));
            /* Solve L^T * x = y */


            /* update alpha = D'*residual */

            tempvec1 = Gsub.block(0,0, m,i)*c.block(0,0, i,1);    /* compute tempvec1 := Gsub*c */


            alpha = DtX.col(signum);                              /* set alpha = D'*x */
            alpha = alpha - tempvec1;                             /* compute alpha := alpha - tempvec1 */

            /* update residual norm */

            if (erroromp) {
                vec_assign(tempvec2, tempvec1, ind, i, 0);   /* assign tempvec2 := tempvec1(ind) */
                delta = (c.block(0,0,i,1).transpose()*tempvec2.block(0,0,i,1))(0,0);       /* compute c'*tempvec2 */
                resnorm = resnorm - delta + deltaprev;       /* residual norm update */
                deltaprev = delta;
                //std::cout << delta << " " << resnorm << std::endl;
            }


        }


        /*** generate output vector gamma ***/

        GammaVector[signum].startFill();
        for (j=0; j<i; ++j) {
            GammaVector[signum].fillrand(ind(j,0),0) = c(j,0);
        }
        GammaVector[signum].endFill();

    }

    /*** allocate output matrix ***/
    Eigen::SparseMatrix<double> Gamma(m,L);
    Gamma.startFill();
    for (signum=0; signum<L; ++signum) {
      for (int k=0; k<GammaVector[signum].outerSize(); ++k)
        for (Eigen::SparseMatrix<double>::InnerIterator it(GammaVector[signum],k); it; ++it)
        {
          Gamma.fillrand(it.row(),signum) = it.value();
        }
    }
    Gamma.endFill();

    /* end omp */
    return Gamma;
}
