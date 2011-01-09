#ifndef TRAINERMAIRAL_H
#define TRAINERMAIRAL_H

#include "sprscode_global.h"
#include "trainer.h"
//#include "vigra/matrix.hxx"


#include <eigen2/Eigen/Core>
#include <eigen2/Eigen/Sparse>
USING_PART_OF_NAMESPACE_EIGEN

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
