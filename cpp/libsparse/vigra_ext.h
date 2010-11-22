#ifndef VIGRA_EXT_H
#define VIGRA_EXT_H

#include "regression.hxx"
// #include <vigra/random.hxx>
#include <iostream>

#include <eigen2/Eigen/Core>
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

template<typename T1,typename T2>
MatrixXf subselect(MatrixBase<T1>& M, MatrixBase<T2>& select, int vars) {
    MatrixXf sub(M.rows(),vars);
    int ii=0;
    for(int i=0; i<select.size(); i++) {
        if(select(i)==1.0) {
          sub.col(ii) = M.col(i);
          ii++;
        }
    }
    return sub;
}

int maxabs(VectorXf& c);
void vec_assign(MatrixXf& y, MatrixXf& x, VectorXi& ind, int k, int start);
////void vec_assign(vigra::Matrix<double>& y, vigra::Matrix<double>& x, vigra::Matrix<int>& ind, int k, int start);
//vigra::Matrix<double> dense_vector(vigra::ArrayVector<int>  active_set, vigra::Matrix<double>  sparse_vector, int size);
//vigra::Matrix<double> lasso(vigra::Matrix<double>& x, vigra::Matrix<double>& D);

#endif // VIGRA_EXT_H
