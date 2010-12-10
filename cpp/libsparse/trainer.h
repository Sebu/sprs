#ifndef TRAINER_H
#define TRAINER_H

class Dictionary;
class Samples;
class Coder;
\
class Trainer
{
public:
    Coder *coder;
    Trainer();
    virtual void train(Samples&, Dictionary& D, int iterations, int batch) = 0;
};

#endif // TRAINER_H
