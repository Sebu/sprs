#ifndef TRAINER_H
#define TRAINER_H

#include "vigra/matrix.hxx"

class Dictionary;
class Samples;

class Trainer
{
public:
    Trainer();
    virtual void train(Samples&, Dictionary& D, int iterations, int batch) = 0;
};

#endif // TRAINER_H
