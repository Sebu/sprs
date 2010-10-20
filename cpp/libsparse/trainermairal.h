#ifndef TRAINERMAIRAL_H
#define TRAINERMAIRAL_H

#include "libsparse_global.h"
#include "trainer.h"
#include "vigra/matrix.hxx"

class LIBSPARSESHARED_EXPORT TrainerMairal : public Trainer
{
public:
    TrainerMairal();
    void train(Samples& samples, Dictionary& D, int iterations);
    void update(vigra::Matrix<double>& A, vigra::Matrix<double>& B, Dictionary& D);
    void pause(); // save A,B matrix
    void save(const char* filename);
    void laod(const char* filename);
};

#endif // TRAINERMAIRAL_H
