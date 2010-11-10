

#ifndef VIGRA_REGRESSION_HXX
#define VIGRA_REGRESSION_HXX

#include <vigra/matrix.hxx>
#include <vigra/linear_solve.hxx>
#include <vigra/singular_value_decomposition.hxx>
#include <vigra/numerictraits.hxx>
#include <vigra/functorexpression.hxx>

#include <iostream>

namespace vigra
{

namespace linalg
{



/** \brief Pass options to leastAngleRegression().

    <b>\#include</b> \<<a href="regression_8hxx-source.html">vigra/regression.hxx</a>\>
        Namespaces: vigra and vigra::linalg
*/
class LeastAngleRegressionOptions
{
  public:
    enum Mode { LARS, LASSO, NNLASSO };

        /** Initialize all options with default values.
        */
    LeastAngleRegressionOptions()
    : max_solution_count(0),
      unconstrained_dimension_count(0),
      mode(LASSO),
      least_squares_solutions(true)
    {}

        /** Maximum number of solutions to be computed.

            If \a n is 0 (the default), the number of solutions is determined by the length
            of the solution array. Otherwise, the minimum of maxSolutionCount() and that
            length is taken.<br>
            Default: 0 (use length of solution array)
        */
    LeastAngleRegressionOptions & maxSolutionCount(unsigned int n)
    {
        max_solution_count = (int)n;
        return *this;
    }

        /** Set the mode of the algorithm.

            Mode must be one of "lars", "lasso", "nnlasso". The function just calls
            the member function of the corresponding name to set the mode.

            Default: "lasso"
        */
    LeastAngleRegressionOptions & setMode(std::string mode)
    {
        for(unsigned int k=0; k<mode.size(); ++k)
            mode[k] = (std::string::value_type)tolower(mode[k]);
        if(mode == "lars")
            this->lars();
        else if(mode == "lasso")
            this->lasso();
        else if(mode == "nnlasso")
            this->nnlasso();
        else
            vigra_fail("LeastAngleRegressionOptions.setMode(): Invalid mode.");
        return *this;
    }


        /** Use the plain LARS algorithm.

            Default: inactive
        */
    LeastAngleRegressionOptions & lars()
    {
        mode = LARS;
        return *this;
    }

        /** Use the LASSO modification of the LARS algorithm.

            This allows features to be removed from the active set under certain conditions.<br>
            Default: active
        */
    LeastAngleRegressionOptions & lasso()
    {
        mode = LASSO;
        return *this;
    }

        /** Use the non-negative LASSO modification of the LARS algorithm.

            This enforces all non-zero entries in the solution to be positive.<br>
            Default: inactive
        */
    LeastAngleRegressionOptions & nnlasso()
    {
        mode = NNLASSO;
        return *this;
    }

        /** Compute least squares solutions.

            Use least angle regression to determine active sets, but
            return least squares solutions for the features in each active set,
            instead of constrained solutions.<br>
            Default: <tt>true</tt>
        */
    LeastAngleRegressionOptions & leastSquaresSolutions(bool select = true)
    {
        least_squares_solutions = select;
        return *this;
    }

    int max_solution_count, unconstrained_dimension_count;
    Mode mode;
    bool least_squares_solutions;
};

namespace detail {

template <class T, class C1, class C2>
struct LarsData
{
    typedef typename MultiArrayShape<2>::type Shape;

    int activeSetSize;
    MultiArrayView<2, T, C1> A;
    MultiArrayView<2, T, C2> b;
    Matrix<T> R, qtb, lars_solution, lars_prediction, next_lsq_solution, next_lsq_prediction, searchVector;
    ArrayVector<MultiArrayIndex> columnPermutation;

    // init data for a new run
    LarsData(MultiArrayView<2, T, C1> const & Ai, MultiArrayView<2, T, C2> const & bi)
    : activeSetSize(1),
      A(Ai), b(bi), R(A), qtb(b),
      lars_solution(A.shape(1), 1), lars_prediction(A.shape(0), 1),
      next_lsq_solution(A.shape(1), 1), next_lsq_prediction(A.shape(0), 1), searchVector(A.shape(0), 1),
      columnPermutation(A.shape(1))
    {
        for(unsigned int k=0; k<columnPermutation.size(); ++k)
            columnPermutation[k] = k;
    }

    // copy data for the recursive call in nnlassolsq
    LarsData(LarsData const & d, int asetSize)
    : activeSetSize(asetSize),
      A(d.R.subarray(Shape(0,0), Shape(d.A.shape(0), activeSetSize))), b(d.qtb), R(A), qtb(b),
      lars_solution(d.lars_solution.subarray(Shape(0,0), Shape(activeSetSize, 1))), lars_prediction(d.lars_prediction),
      next_lsq_solution(d.next_lsq_solution.subarray(Shape(0,0), Shape(activeSetSize, 1))), 
      next_lsq_prediction(d.next_lsq_prediction), searchVector(d.searchVector),
      columnPermutation(A.shape(1))
    {
        for(unsigned int k=0; k<columnPermutation.size(); ++k)
            columnPermutation[k] = k;
    }
};

template <class T, class C1, class C2, class Array1, class Array2>
unsigned int leastAngleRegressionMainLoop(LarsData<T, C1, C2> & d,
                  Array1 & activeSets, Array2 * lars_solutions, Array2 * lsq_solutions,
                  LeastAngleRegressionOptions const & options)
{
    using namespace vigra::functor;

    typedef typename MultiArrayShape<2>::type Shape;
    typedef typename Matrix<T>::view_type Subarray;
    typedef ArrayVector<MultiArrayIndex> Permutation;
    typedef typename Permutation::view_type ColumnSet;

    vigra_precondition(d.activeSetSize > 0,
       "leastAngleRegressionMainLoop() must not be called with empty active set.");

    bool enforce_positive = (options.mode == LeastAngleRegressionOptions::NNLASSO);
    bool lasso_modification = (options.mode != LeastAngleRegressionOptions::LARS);

    const MultiArrayIndex rows = rowCount(d.R);
    const MultiArrayIndex cols = columnCount(d.R);
    const MultiArrayIndex maxRank = std::min(rows, cols);

    MultiArrayIndex maxSolutionCount = options.max_solution_count;
    if(maxSolutionCount == 0)
        maxSolutionCount = lasso_modification
                                ? 10*maxRank
                                : maxRank;

    bool needToRemoveColumn = false;
    MultiArrayIndex columnToBeAdded, columnToBeRemoved;
    MultiArrayIndex currentSolutionCount = 0;
    while(currentSolutionCount < maxSolutionCount)
    {
        ColumnSet activeSet = d.columnPermutation.subarray(0, (unsigned int)d.activeSetSize);
        ColumnSet inactiveSet = d.columnPermutation.subarray((unsigned int)d.activeSetSize, (unsigned int)cols);

        // find next dimension to be activated
        Matrix<T> cLARS = transpose(d.A) * (d.b - d.lars_prediction),      // correlation with LARS residual
                  cLSQ  = transpose(d.A) * (d.b - d.next_lsq_prediction);  // correlation with LSQ residual

        // In theory, all vectors in the active set should have the same correlation C, and
        // the correlation of all others should not exceed this. In practice, we may find the
        // maximum correlation in any variable due to tiny numerical inaccuracies. Therefore, we
        // determine C from the entire set of variables.
        MultiArrayIndex cmaxIndex = enforce_positive
                                      ? argMax(cLARS)
                                      : argMax(abs(cLARS));
        T C = abs(cLARS(cmaxIndex, 0));

        Matrix<T> ac(cols - d.activeSetSize, 1);
        for(MultiArrayIndex k = 0; k<cols-d.activeSetSize; ++k)
        {
            T rho = cLSQ(inactiveSet[k], 0), 
              cc  = C - sign(rho)*cLARS(inactiveSet[k], 0);

            if(rho == 0.0)  // make sure that 0/0 cannot happen in the other cases
                ac(k,0) = 1.0; // variable k is linearly dependent on the active set
            else if(rho > 0.0)
                ac(k,0) = cc / (cc + rho); // variable k would enter the active set with positive sign
            else if(enforce_positive)
                ac(k,0) = 1.0; // variable k cannot enter the active set because it would be negative
            else
                ac(k,0) = cc / (cc - rho); // variable k would enter the active set with negative sign
        }

        // in the non-negative case: make sure that a column just removed cannot re-enter right away
        // (in standard LASSO, this is allowed, because the variable may re-enter with opposite sign)
        if(enforce_positive && needToRemoveColumn)
                ac(columnToBeRemoved-d.activeSetSize,0) = 1.0;

        // find candidate
        // Note: R uses Arg1() > epsilon, but this is only possible because it allows several variables to
        //       join the active set simultaneously, so that gamma = 0 cannot occur.
        columnToBeAdded = argMin(ac);

        // if no new column can be added, we do a full step gamma = 1.0 and then stop, unless a column is removed below
        T gamma = (d.activeSetSize == maxRank)
                     ? 1.0
                     : ac(columnToBeAdded, 0);

        // adjust columnToBeAdded: we skipped the active set
        if(columnToBeAdded >= 0)
            columnToBeAdded += d.activeSetSize;

        // check whether we have to remove a column from the active set
        needToRemoveColumn = false;
        if(lasso_modification)
        {
            // find dimensions whose weight changes sign below gamma*searchDirection
            Matrix<T> s(Shape(d.activeSetSize, 1), NumericTraits<T>::max());
            for(MultiArrayIndex k=0; k<d.activeSetSize; ++k)
            {
                if(( enforce_positive && d.next_lsq_solution(k,0) < 0.0) ||
                   (!enforce_positive && sign(d.lars_solution(k,0))*sign(d.next_lsq_solution(k,0)) == -1.0))
                        s(k,0) = d.lars_solution(k,0) / (d.lars_solution(k,0) - d.next_lsq_solution(k,0));
            }

            columnToBeRemoved = argMinIf(s, Arg1() <= Param(gamma));
            if(columnToBeRemoved >= 0)
            {
                needToRemoveColumn = true; // remove takes precedence over add
                gamma = s(columnToBeRemoved, 0);
            }
        }

        // compute the current solutions
        d.lars_prediction  = gamma * d.next_lsq_prediction + (1.0 - gamma) * d.lars_prediction;
        d.lars_solution    = gamma * d.next_lsq_solution   + (1.0 - gamma) * d.lars_solution;
        if(needToRemoveColumn)
            d.lars_solution(columnToBeRemoved, 0) = 0.0;  // turn possible epsilon into an exact zero

        // write the current solution
        ++currentSolutionCount;
        activeSets.push_back(typename Array1::value_type(d.columnPermutation.begin(), d.columnPermutation.begin()+d.activeSetSize));

        if(lsq_solutions != 0)
        {
            if(enforce_positive)
            {
                ArrayVector<Matrix<T> > nnresults;
	            ArrayVector<ArrayVector<MultiArrayIndex> > nnactiveSets;
	            LarsData<T, C1, C2> nnd(d, d.activeSetSize);

                leastAngleRegressionMainLoop(nnd, nnactiveSets, &nnresults, (Array2*)0,
                                             LeastAngleRegressionOptions().leastSquaresSolutions(false).nnlasso());
                Matrix<T> nnlsq_solution(d.activeSetSize, 1);
                for(unsigned int k=0; k<nnactiveSets.back().size(); ++k)
                {
                    nnlsq_solution(nnactiveSets.back()[k],0) = nnresults.back()[k];
                }
                lsq_solutions->push_back(nnlsq_solution);
            }
            else
            {
                lsq_solutions->push_back(d.next_lsq_solution.subarray(Shape(0,0), Shape(d.activeSetSize, 1)));
            }
        }
        if(lars_solutions != 0)
        {
            lars_solutions->push_back(d.lars_solution.subarray(Shape(0,0), Shape(d.activeSetSize, 1)));
        }

        // no further solutions possible
        if(gamma == 1.0)
            break;

        if(needToRemoveColumn)
        {
            --d.activeSetSize;
            if(columnToBeRemoved != d.activeSetSize)
            {
                // remove column 'columnToBeRemoved' and restore triangular form of R
                // note: columnPermutation is automatically swapped here
                detail::upperTriangularSwapColumns(columnToBeRemoved, d.activeSetSize, d.R, d.qtb, d.columnPermutation);

                // swap solution entries
                std::swap(d.lars_solution(columnToBeRemoved, 0), d.lars_solution(d.activeSetSize,0));
                std::swap(d.next_lsq_solution(columnToBeRemoved, 0), d.next_lsq_solution(d.activeSetSize,0));
                columnToBeRemoved = d.activeSetSize; // keep track of removed column
            }
            d.lars_solution(d.activeSetSize,0) = 0.0;
            d.next_lsq_solution(d.activeSetSize,0) = 0.0;
        }
        else
        {
            vigra_invariant(columnToBeAdded >= 0,
                "leastAngleRegression(): internal error (columnToBeAdded < 0)");
            // add column 'columnToBeAdded'
            if(d.activeSetSize != columnToBeAdded)
            {
                std::swap(d.columnPermutation[d.activeSetSize], d.columnPermutation[columnToBeAdded]);
                columnVector(d.R, d.activeSetSize).swapData(columnVector(d.R, columnToBeAdded));
                columnToBeAdded = d.activeSetSize; // keep track of added column
            }

            // zero the corresponding entries of the solutions
            d.next_lsq_solution(d.activeSetSize,0) = 0.0;
            d.lars_solution(d.activeSetSize,0) = 0.0;

            // reduce R (i.e. its newly added column) to triangular form
            detail::qrColumnHouseholderStep(d.activeSetSize, d.R, d.qtb);
            ++d.activeSetSize;
        }

        // compute the LSQ solution of the new active set
        Subarray Ractive = d.R.subarray(Shape(0,0), Shape(d.activeSetSize, d.activeSetSize));
        Subarray qtbactive = d.qtb.subarray(Shape(0,0), Shape(d.activeSetSize, 1));
        Subarray next_lsq_solution_view = d.next_lsq_solution.subarray(Shape(0,0), Shape(d.activeSetSize, 1));
        linearSolveUpperTriangular(Ractive, qtbactive, next_lsq_solution_view);

        // compute the LSQ prediction of the new active set
        d.next_lsq_prediction.init(0.0);
        for(MultiArrayIndex k=0; k<d.activeSetSize; ++k)
            d.next_lsq_prediction += next_lsq_solution_view(k,0)*columnVector(d.A, d.columnPermutation[k]);
    }

    return (unsigned int)currentSolutionCount;
}


template <class T, class C1, class C2, class Array1, class Array2>
unsigned int
leastAngleRegressionImpl(MultiArrayView<2, T, C1> const & A, MultiArrayView<2, T, C2> const &b,
                         Array1 & activeSets, Array2 * lasso_solutions, Array2 * lsq_solutions,
                         LeastAngleRegressionOptions const & options)
{
    using namespace vigra::functor;

    const MultiArrayIndex rows = rowCount(A);

    vigra_precondition(rowCount(b) == rows && columnCount(b) == 1,
       "leastAngleRegression(): Shape mismatch between matrices A and b.");

    bool enforce_positive = (options.mode == LeastAngleRegressionOptions::NNLASSO);

    detail::LarsData<T, C1, C2> d(A, b);

    // find dimension with largest correlation
    Matrix<T> c = transpose(A)*b;
    MultiArrayIndex initialColumn = enforce_positive
                                       ? argMaxIf(c, Arg1() > Param(0.0))
                                       : argMax(abs(c));
    if(initialColumn == -1)
        return 0; // no solution found

    // prepare initial active set and search direction etc.
    std::swap(d.columnPermutation[0], d.columnPermutation[initialColumn]);
    columnVector(d.R, 0).swapData(columnVector(d.R, initialColumn));
    detail::qrColumnHouseholderStep(0, d.R, d.qtb);
    d.next_lsq_solution(0,0) = d.qtb(0,0) / d.R(0,0);
    d.next_lsq_prediction = d.next_lsq_solution(0,0) * columnVector(A, d.columnPermutation[0]);
    d.searchVector = d.next_lsq_solution(0,0) * columnVector(A, d.columnPermutation[0]);

    return leastAngleRegressionMainLoop(d, activeSets, lasso_solutions, lsq_solutions, options);
}


} // namespace detail



template <class T, class C1, class C2, class Array1, class Array2>
inline unsigned int
leastAngleRegression(MultiArrayView<2, T, C1> const & A, MultiArrayView<2, T, C2> const &b,
                     Array1 & activeSets, Array2 & solutions,
                     LeastAngleRegressionOptions const & options = LeastAngleRegressionOptions())
{
    if(options.least_squares_solutions)
        return detail::leastAngleRegressionImpl(A, b, activeSets, (Array2*)0, &solutions, options);
    else
        return detail::leastAngleRegressionImpl(A, b, activeSets, &solutions, (Array2*)0, options);
}

template <class T, class C1, class C2, class Array1, class Array2>
inline unsigned int
leastAngleRegression(MultiArrayView<2, T, C1> const & A, MultiArrayView<2, T, C2> const &b,
                     Array1 & activeSets, Array2 & lasso_solutions, Array2 & lsq_solutions,
                     LeastAngleRegressionOptions const & options = LeastAngleRegressionOptions())
{
    return detail::leastAngleRegressionImpl(A, b, activeSets, &lasso_solutions, &lsq_solutions, options);
}



} // namespace linalg

using linalg::leastAngleRegression;
using linalg::LeastAngleRegressionOptions;

} // namespace vigra

#endif // VIGRA_REGRESSION_HXX
