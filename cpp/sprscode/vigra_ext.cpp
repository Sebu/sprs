#include "vigra_ext.h"

int maxabs(VectorXd& c)
{
  int maxid=1, k;
  int m = c.rows();
  double absval, maxval = 0;

  for (k=0; k<m; ++k) {
    absval = fabs(c(k));
    if (absval > maxval) {
      maxval = absval;
      maxid = k;
    }
  }
  return maxid;
}


void vec_assign(MatrixXd& y, MatrixXd& x, VectorXi& ind, int k, int start)
{
  int i;
  for (i=0; i<k; ++i)
    y(i,0) = x(ind(i), start);
}

cv::Mat inshape(cv::Mat& M, int dim, int channels) {
    ///cv::Mat M = in.reshape(1);
    cv::Mat result(dim, dim*channels, CV_8U);

    int i = 0, j = 0;
    int d = -1; // -1 for top-right move, +1 for bottom-left move
    int start = 0, end = dim * dim - 1;
    do
    {
        result.at<uchar>(i, j*channels) =   M.at<uchar>(0,start*channels);
        result.at<uchar>(i, j*channels+1) = M.at<uchar>(0,start*channels+1);
        result.at<uchar>(i, j*channels+2) = M.at<uchar>(0,start*channels+2);
        start++;
//        result[i, j] = start++;

        result.at<uchar>(dim-i-1, (dim-j-1)*channels) =   M.at<uchar>(0,end*channels);
        result.at<uchar>(dim-i-1, (dim-j-1)*channels+1) = M.at<uchar>(0,end*channels+1);
        result.at<uchar>(dim-i-1, (dim-j-1)*channels+2) = M.at<uchar>(0,end*channels+2);
        end--;
//        result[n - i - 1, n - j - 1] = end--;

        i += d; j -= d;
        if (i < 0)
        {
            i++; d = -d; // top reached, reverse
        }
        else if (j < 0)
        {
            j++; d = -d; // left reached, reverse
        }
    } while (start < end);
    if (start == end) {
        result.at<uchar>(i, j*channels) =   M.at<uchar>(0,start*channels);
        result.at<uchar>(i, j*channels+1) = M.at<uchar>(0,start*channels+1);
        result.at<uchar>(i, j*channels+2) = M.at<uchar>(0,start*channels+2);
//        result[i, j] = start;
    }
    return result.reshape(channels);
}

cv::Mat unshape(cv::Mat& in, int dim, int channels) {
    cv::Mat M = in.reshape(1);
    cv::Mat result(1, dim*dim*channels, CV_8U);

    int i = 0, j = 0;
    int d = -1; // -1 for top-right move, +1 for bottom-left move
    int start = 0, end = dim * dim - 1;
    do
    {
        result.at<uchar>(0,start*channels) =   M.at<uchar>(i, j*channels);
        start++;
//        result[i, j] = start++;

        result.at<uchar>(0,end*channels) =   M.at<uchar>(dim-i-1, (dim-j-1)*channels);
        end--;
//        result[n - i - 1, n - j - 1] = end--;

        i += d; j -= d;
        if (i < 0)
        {
            i++; d = -d; // top reached, reverse
        }
        else if (j < 0)
        {
            j++; d = -d; // left reached, reverse
        }
    } while (start < end);
    if (start == end) {
        result.at<uchar>(0,start*channels) =   M.at<uchar>(i, j*channels);
//        result[i, j] = start;
    }
    return result;
}



//vigra::Matrix<double> dense_vector(vigra::ArrayVector<int>  active_set, vigra::Matrix<double>  sparse_vector, int size) {

//    vigra::Matrix<double> dense_vector(size,1);
//    dense_vector.init(0.0);
//    for (unsigned int i = 0; i < active_set.size(); i++)
//        dense_vector(active_set[i],0) = sparse_vector(i,0);

//    return dense_vector;
//}

//vigra::Matrix<double> lasso(vigra::Matrix<double>& x, vigra::Matrix<double>& D) {

//    int bestIndex = 0;
//    vigra::ArrayVector<vigra::ArrayVector<int> > active_sets;
//    vigra::ArrayVector<vigra::Matrix<double> > solutions;


//    vigra::linalg::LeastAngleRegressionOptions opts;
//    opts.lasso();
//    opts.maxSolutionCount(10);
//    // run leastAngleRegression() in  LASSO mode
//    int numSolutions = vigra::linalg::leastAngleRegression(D, x, active_sets, solutions, opts);


//    //std::cout << bestIndex << std::endl;

////    for (MultiArrayIndex k = 0; k < numSolutions; ++k) {
////        Matrix<double> dense_solution = dense_vector(active_sets[k], solutions[k], D.columnCount());
////        double lsq = (mmul(D,dense_solution)-x).squaredNorm();
////        //        std::cout <<  k << " " << sum(solutions[k]) <<  " " << solutions[k].size() << std::endl;
////        double error = 0.5*lsq + sum(solutions[k]);
////        if(error<bestError) { bestError=error; bestIndex=k; }
////    }

//    bestIndex = numSolutions - 1;
//    return dense_vector(active_sets[bestIndex], solutions[bestIndex], D.columnCount());
//}
