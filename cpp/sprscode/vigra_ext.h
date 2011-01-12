#ifndef VIGRA_EXT_H
#define VIGRA_EXT_H

#include "regression.hxx"
#include <opencv/cv.h>
#include <iostream>
#include <vector>

#include <eigen2/Eigen/Core>
#include <eigen2/Eigen/Array>

USING_PART_OF_NAMESPACE_EIGEN

//template <class T, class C1, class C2, class C3>
//bool linearSolveUpperTriangularC(const vigra::MultiArrayView<2, T, C1> &r, const vigra::MultiArrayView<2, T, C2> &b,
//                                vigra::MultiArrayView<2, T, C3> x)
//{
//    typedef vigra::MultiArrayShape<2>::type Shape;
//    vigra::MultiArrayIndex m = rowCount(r);
//    vigra::MultiArrayIndex rhsCount = columnCount(b);
////    vigra_precondition(m == columnCount(r),
////        "linearSolveUpperTriangular(): square coefficient matrix required.");
////    vigra_precondition(m == rowCount(b) && m == rowCount(x) && rhsCount == columnCount(x),
////        "linearSolveUpperTriangular(): matrix shape mismatch.");

//    for(vigra::MultiArrayIndex k = 0; k < rhsCount; ++k)
//    {
//        for(int i=m-1; i>=0; --i)
//        {
//            if(r(i,i) == vigra::NumericTraits<T>::zero())
//                return false;  // r doesn't have full rank
//            T sum = b(i, k);
//            for(vigra::MultiArrayIndex j=i+1; j<m; ++j)
//                 sum -= r(i, j) * x(j, k);
//            x(i, k) = sum / r(i, i);
//        }
//    }
//    return true;
//}


//template <class T, class C1, class C2, class C3>
//bool linearSolveLowerTriangularC(const vigra::MultiArrayView<2, T, C1> &l, const vigra::MultiArrayView<2, T, C2> &b,
//                            vigra::MultiArrayView<2, T, C3> x)
//{
//    vigra::MultiArrayIndex m = columnCount(l);
//    vigra::MultiArrayIndex n = columnCount(b);
////    vigra_precondition(m == rowCount(l),
////        "linearSolveLowerTriangular(): square coefficient matrix required.");
////    vigra_precondition(m == rowCount(b) && m == rowCount(x) && n == columnCount(x),
////        "linearSolveLowerTriangular(): matrix shape mismatch.");

//    for(vigra::MultiArrayIndex k = 0; k < n; ++k)
//    {
//        for(vigra::MultiArrayIndex i=0; i<m; ++i)
//        {
//            if(l(i,i) == vigra::NumericTraits<T>::zero())
//                return false;  // l doesn't have full rank
//            T sum = b(i, k);
//            for(vigra::MultiArrayIndex j=0; j<i; ++j)
//                 sum -= l(i, j) * x(j, k);
//            x(i, k) = sum / l(i, i);
//        }
//    }
//    return true;
//}

//template <class T, class C1, class C2, class C3>
//inline
//void choleskySolveC(vigra::MultiArrayView<2, T, C1> const & L, vigra::MultiArrayView<2, T, C2> const & b, vigra::MultiArrayView<2, T, C3> & x)
//{
//    /* Solve L * y = b */
//    linearSolveLowerTriangularC(L, b, x);
//    /* Solve L^T * x = y */
//    linearSolveUpperTriangularC(transpose(L), x, x);
//}

//template<class T, class C>
//T sum(vigra::MultiArrayView< 2, T, C > const & m) {
//    T result = 0;
//    for(int i=0; i<m.size(0); i++)
//        for(int j=0; j<m.size(1); j++)
//            result += m(i,j);

//    return result;
//}

//template<class T, class C>
//void init_random(vigra::MultiArrayView< 2, T, C >& m) {
//    for(int i=0; i<m.size(0); i++)
//        for(int j=0; j<m.size(1); j++)
//            m(i,j) = rand();
//}

template<typename T1>
MatrixXd subselect(MatrixBase<T1>& M, std::vector<int>& select) {
    MatrixXd sub(M.rows(),select.size());
    for(int i=0; i<select.size(); i++) {
       sub.col(i) = M.col(select[i]);
    }
    return sub;
}

template<typename T1, typename T2>
void unshift(MatrixBase<T1>& M, MatrixBase<T2>& shift) {
  for(int i=0; i<M.cols();i++) {
//    std::cout << i << std::endl;
    M.col(i).cwise() += shift(i);
  }
}

template<typename T1>
MatrixXd center(MatrixBase<T1>& M) {
    VectorXd shift(M.cols());
    for(int i=0; i<M.cols();i++) {
        shift(i) = M.col(i).sum()/M.rows();
        M.col(i).cwise() -= shift(i);
    }
    return shift;
}

template<typename T1>
void divVariance(MatrixBase<T1>& M) {
    for(int i=0; i<M.cols();i++) {
        double var = M.col(i).squaredNorm(); ///M.rows();
        if (var==0.0) continue;
        for(int j=0; j<M.rows();j++)
          M(j,i) /= var;
    }
}


template<typename T1>
double mse(MatrixBase<T1>& I, MatrixBase<T1>& K) {
    double sum = (I - K).squaredNorm();
    return sum/(I.cols()*I.rows());
}

inline double psnr(double mseVal) {
    return 10 * std::log10( (255*255)/ mseVal );
}

template<typename T1>
double psnr(MatrixBase<T1>& I, MatrixBase<T1>& K) {
    double mseVal = mse(I,K);
    return psnr(mseVal);

}



int maxabs(VectorXd& c);
cv::Mat unshape(cv::Mat& in, int dim, int channels);
cv::Mat inshape(cv::Mat& M, int dim, int channels);
void vec_assign(MatrixXd& y, MatrixXd& x, VectorXi& ind, int k, int start);
////void vec_assign(vigra::Matrix<double>& y, vigra::Matrix<double>& x, vigra::Matrix<int>& ind, int k, int start);
//vigra::Matrix<double> dense_vector(vigra::ArrayVector<int>  active_set, vigra::Matrix<double>  sparse_vector, int size);
//vigra::Matrix<double> lasso(vigra::Matrix<double>& x, vigra::Matrix<double>& D);

#endif // VIGRA_EXT_H
