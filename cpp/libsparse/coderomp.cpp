#include "coderomp.h"

#include <math.h>
#include <string.h>
#include <vigra/matrix.hxx>
#include <vigra/linear_algebra.hxx>

CoderOMP::CoderOMP()
{
}

vigra::Matrix<double> CoderOMP::code(vigra::Matrix<double>& X, vigra::Matrix<double>& D, int n, int m, int L,
                 int T, double eps, int erroromp)
{
    int i, j, signum, pos;
    double eps2, resnorm, delta, deltaprev;

    /* precalculate for speed */
    vigra::Matrix<double> DtX = D.transpose()*X;
    vigra::Matrix<double> XtX = X.transpose()*X;
    vigra::Matrix<double> G = D.transpose()*D;

    /*** allocate output matrix ***/
    vigra::Matrix<double> Gamma (m,L);

    /*** helper arrays ***/
    vigra::Matrix<double> alpha;            /* contains D'*residual */
    vigra::Matrix<double> c;                /* orthogonal projection result */
    vigra::Matrix<int> ind(n,1);            /* indices of selected atoms */
    vigra::Matrix<int> selected_atoms(m,1); /* binary array with 1's for selected atoms */

    /* current number of columns in Dsub / Gsub / Lchol */
    int allocated_cols = erroromp ? (int)(ceil(sqrt((double)n)/2.0) + 1.01) : T;

    /* Cholesky decomposition of D_I'*D_I */
    vigra::Matrix<double> Lchol(n,allocated_cols);

    /* temporary vectors for various computations */
    vigra::Matrix<double> tempvec1;
    vigra::Matrix<double> tempvec2;

    /* matrix containing G(:,ind) - the columns of G corresponding to the selected atoms, in order of selection */
    vigra::Matrix<double> Gsub(m,allocated_cols);

    /*** initializations for error omp ***/
    if (erroromp) {
        eps2 = eps*eps;        /* compute eps^2 */
        if (T<0 || T>n) {      /* unspecified max atom num - set max atoms to n */
            T = n;
        }
    }


    /**********************   perform omp for each signal   **********************/
    for (signum=0; signum<L; ++signum) {

        /* initialize residual norm and deltaprev for error-omp */

        if (erroromp) {
            resnorm = XtX(signum,1);
            deltaprev = 0;     /* delta tracks the value of gamma'*G*gamma */
        }
        else {
            /* ignore residual norm stopping criterion */
            eps2 = 0;
            resnorm = 1;
        }


        if (resnorm>eps2 && T>0) {

            /* initialize alpha := DtX */
            alpha = DtX.columnVector(signum);

            /* mark all atoms as unselected */
            for (i=0; i<m; ++i) {
                selected_atoms(i,1) = 0;
            }

        }

        /* main loop */
        i=0;
        while (resnorm>eps2 && i<T) {

            /* TODO: index of next atom */
            //pos = maxabs(alpha, m);

            /* stop criterion: selected same atom twice, or inner product too small */

            if (selected_atoms(pos,1) || alpha(pos,1)*alpha(pos,1)<1e-14) {
                break;
            }


            /* mark selected atom */

            ind(i,1) = pos;
            selected_atoms(pos,1) = 1;


            /* matrix reallocation */

//            if (erroromp && i>=allocated_cols) {

//                allocated_cols = (int)(ceil(allocated_cols*MAT_INC_FACTOR) + 1.01);

//                Lchol = (double*)mxRealloc(Lchol,n*allocated_cols*sizeof(double));

//                Gsub = (double*)mxRealloc(Gsub,m*allocated_cols*sizeof(double));
//            }


            /* append column to Gsub or Dsub */
            memcpy(Gsub+i*m, G+pos*m, m*sizeof(double));

            /*** Cholesky update ***/

            if (i==0) {
                Lchol(1,1) = 1;
            }
            else {

                /* incremental Cholesky decomposition: compute next row of Lchol */

                //tempvec1 = Gsub.columnVector(ind(i,1));            /* extract tempvec1 := Gsub(ind,i) */

                /* compute tempvec2 = Lchol \ tempvec1 */
                vigra::linalg::linearSolveLowerTriangular(Lchol, tempvec1, tempvec2);

                for (j=0; j<i; ++j) {                              /* write tempvec2 to end of Lchol */
                    Lchol(i,j) = tempvec2(j,1);
                }
                /* compute Lchol(i,i) */
                Lchol(i,i) = sqrt(1-tempvec2.transpose()*tempvec2);
            }

            i++;

            /* perform orthogonal projection and compute sparse coefficients */
            //tempvec1 = DtX.columnVector(signum);               /* extract tempvec1 = DtX(ind) */
            vigra::linalg::choleskySolve(Lchol,tempvec1,c);      /* solve LL'c = tempvec1 for c */


            /* update alpha = D'*residual */

            tempvec1 = Gsub*c;                                    /* compute tempvec1 := Gsub*c */
            alpha = DtX.columnVector(signum);                     /* set alpha = D'*x */
            alpha = alpha - tempvec1;                             /* compute alpha := alpha - tempvec1 */

            /* update residual norm */
            if (erroromp) {
                //tempvec2 = tempvec1;                          /* assign tempvec2 := tempvec1(ind) */
                delta = c.transpose()*tempvec2;              /* compute c'*tempvec2 */
                resnorm = resnorm - delta + deltaprev;       /* residual norm update */
                deltaprev = delta;
            }

        }


        /*** generate output vector gamma ***/
        for (j=0; j<i; ++j) {
            Gamma(ind(j,1),signum) = c(j,1);
        }

    }

    /* end omp */

    return Gamma;
}

double* ompcore(double DtX[], double XtX[], double G[], int n, int m, int L,
                 int T, double eps, int erroromp)
{
    double *Gamma;
    int i, j, signum, pos, *ind, *gammaIr, *gammaJc;
    int  allocated_cols;
    int *selected_atoms;
    double *alpha, *Lchol, *c, *Gsub, sum, *gammaPr, *tempvec1, *tempvec2;
    double eps2, resnorm, delta, deltaprev;



    /*** allocate output matrix ***/
    Gamma = malloc(m*L,*sizeof(double));
    gammaPr = Gamma;
    gammaIr = 0;
    gammaJc = 0;


    /*** helper arrays ***/
    alpha = (double*)malloc(m*sizeof(double));        /* contains D'*residual */
    ind = (int*)malloc(n*sizeof(int));        /* indices of selected atoms */
    selected_atoms = (int*)malloc(m*sizeof(int));     /* binary array with 1's for selected atoms */
    c = (double*)malloc(n*sizeof(double));            /* orthogonal projection result */

    /* current number of columns in Dsub / Gsub / Lchol */
    allocated_cols = erroromp ? (int)(ceil(sqrt((double)n)/2.0) + 1.01) : T;

    /* Cholesky decomposition of D_I'*D_I */
    Lchol = (double*)malloc(n*allocated_cols*sizeof(double));

    /* temporary vectors for various computations */
    tempvec1 = (double*)malloc(m*sizeof(double));
    tempvec2 = (double*)malloc(m*sizeof(double));


    /* matrix containing G(:,ind) - the columns of G corresponding to the selected atoms, in order of selection */
    Gsub = (double*)malloc(m*allocated_cols*sizeof(double));



    /*** initializations for error omp ***/
    if (erroromp) {
        eps2 = eps*eps;        /* compute eps^2 */
        if (T<0 || T>n) {      /* unspecified max atom num - set max atoms to n */
            T = n;
        }
    }


    /**********************   perform omp for each signal   **********************/
    for (signum=0; signum<L; ++signum) {


        /* initialize residual norm and deltaprev for error-omp */

        if (erroromp) {
            resnorm = XtX[signum];
            deltaprev = 0;     /* delta tracks the value of gamma'*G*gamma */
        }
        else {
            /* ignore residual norm stopping criterion */
            eps2 = 0;
            resnorm = 1;
        }


        if (resnorm>eps2 && T>0) {

            /* initialize alpha := DtX */
            memcpy(alpha, DtX + m*signum, m*sizeof(double));

            /* mark all atoms as unselected */
            for (i=0; i<m; ++i) {
                selected_atoms[i] = 0;
            }

        }


        /* main loop */

        i=0;
        while (resnorm>eps2 && i<T) {

            /* index of next atom */

            pos = maxabs(alpha, m);


            /* stop criterion: selected same atom twice, or inner product too small */

            if (selected_atoms[pos] || alpha[pos]*alpha[pos]<1e-14) {
                break;
            }


            /* mark selected atom */

            ind[i] = pos;
            selected_atoms[pos] = 1;


            /* matrix reallocation */

            if (erroromp && i>=allocated_cols) {

                allocated_cols = (int)(ceil(allocated_cols*MAT_INC_FACTOR) + 1.01);

                Lchol = (double*)mxRealloc(Lchol,n*allocated_cols*sizeof(double));

                Gsub = (double*)mxRealloc(Gsub,m*allocated_cols*sizeof(double));
            }


            /* append column to Gsub or Dsub */

            memcpy(Gsub+i*m, G+pos*m, m*sizeof(double));



            /*** Cholesky update ***/

            if (i==0) {
                *Lchol = 1;
            }
            else {

                /* incremental Cholesky decomposition: compute next row of Lchol */

                vec_assign(tempvec1, Gsub+i*m, ind, i);          /* extract tempvec1 := Gsub(ind,i) */

                backsubst('L', Lchol, tempvec1, tempvec2, n, i);   /* compute tempvec2 = Lchol \ tempvec1 */
                for (j=0; j<i; ++j) {                              /* write tempvec2 to end of Lchol */
                    Lchol[j*n+i] = tempvec2[j];
                }

                /* compute Lchol(i,i) */
                sum = 0;
                for (j=0; j<i; ++j) {         /* compute sum of squares of last row without Lchol(i,i) */
                    sum += SQR(Lchol[j*n+i]);
                }
                if ( (1-sum) <= 1e-14 ) {     /* Lchol(i,i) is zero => selected atoms are dependent */
                    break;
                }
                Lchol[i*n+i] = sqrt(1-sum);
            }

            i++;


            /* perform orthogonal projection and compute sparse coefficients */

            vec_assign(tempvec1, DtX + m*signum, ind, i);   /* extract tempvec1 = DtX(ind) */
            cholsolve('L', Lchol, tempvec1, c, n, i);                     /* solve LL'c = tempvec1 for c */


            /* update alpha = D'*residual */

            mat_vec(1, Gsub, c, tempvec1, m, i);                              /* compute tempvec1 := Gsub*c */
            memcpy(alpha, DtX + m*signum, m*sizeof(double));    /* set alpha = D'*x */
            vec_sum(-1, tempvec1, alpha, m);                                  /* compute alpha := alpha - tempvec1 */

            /* update residual norm */
            if (erroromp) {
                vec_assign(tempvec2, tempvec1, ind, i);      /* assign tempvec2 := tempvec1(ind) */
                delta = dotprod(c,tempvec2,i);               /* compute c'*tempvec2 */
                resnorm = resnorm - delta + deltaprev;       /* residual norm update */
                deltaprev = delta;
            }

        }


        /*** generate output vector gamma ***/

        if (gamma_mode == FULL_GAMMA) {    /* write the coefs in c to their correct positions in gamma */
            for (j=0; j<i; ++j) {
                gammaPr[m*signum + ind[j]] = c[j];
            }
        }



    }

    /* end omp */

    /* free memory */
    free(Gsub);
    free(tempvec2);
    free(tempvec1);
    free(Lchol);
    free(c);
    free(selected_atoms);
    free(ind);
    free(alpha);

    return Gamma;
}
