#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <vector>
#include "libsparse_global.h"
#include "vigra_ext.h"

//class DEntry {
//public:
//    int numElements_;
//    float* elements_;

//    DEntry( int numElements ) : numElements_(numElements) {
//        elements_ = new float[numElements_];
//    }

//    ~DEntry() {
//        delete[] elements_;
//    }
//};

class LIBSPARSESHARED_EXPORT Dictionary {
private:
   // std::vector<DEntry*> elements_;
    vigra::Matrix<double>* data_;
    int signalSize_;
    int elementCount_;
    int channels_;
    int size_;
public:
    Dictionary(int, int, int);
   // DEntry* getFirst();
   // DEntry* getLast();
    const vigra::Matrix<double> & getData();
    void initRandom();
    void initFromData(Matrix<double> & data);
    void update(Matrix<double>&, Matrix<double>&);
    void learn(Matrix<double>&, int);
    void debugSaveImage(char* filename);
    void save(char* filename);
    void load(char* filename);
};

#endif // DICTIONARY_H
