#ifndef TRAINERMAIRAL_H
#define TRAINERMAIRAL_H

#include "sprscode_global.h"
#include "trainer.h"
//#include "vigra/matrix.hxx"


#include <Eigen/Core>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>

//USING_PART_OF_NAMESPACE_EIGEN
using namespace Eigen;

class LIBSPARSESHARED_EXPORT TrainerMairal : public Trainer
{
private:
    MatrixXd* A_;
    MatrixXd* B_;
public:
    TrainerMairal();
    void train(Samples& samples, Dictionary& D, int iterations, int batch=1000);
    void update(MatrixXd& A, MatrixXd& B, Dictionary& D);
    void hibernate(); // save A,B matrix
    void save(const char* filename);
    void load(const char* filename);
};

#endif // TRAINERMAIRAL_H
