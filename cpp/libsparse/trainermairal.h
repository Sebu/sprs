#ifndef TRAINERMAIRAL_H
#define TRAINERMAIRAL_H

#include "libsparse_global.h"
#include "trainer.h"
#include "vigra/matrix.hxx"

class LIBSPARSESHARED_EXPORT TrainerMairal : public Trainer
{
private:
    vigra::Matrix<double>* A_;
    vigra::Matrix<double>* B_;
public:
    TrainerMairal();
    void train(Samples& samples, Dictionary& D, int iterations, int batch=1000);
    void update(vigra::Matrix<double>& A, vigra::Matrix<double>& B, Dictionary& D);
    void hibernate(); // save A,B matrix
    void save(const char* filename);
    void load(const char* filename);
};

#endif // TRAINERMAIRAL_H
