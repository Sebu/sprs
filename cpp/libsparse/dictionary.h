#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <vector>
#include "libsparse_global.h"


class DEntry {
public:
    int numElements_;
    float* elements_;

    void DEntry( int numElements ) : numElements_(numElements) {
        float* elements_ = new float[numElements_];
    }

    void ~DEntry() {
        delete[] elements_;
    }
};

class LIBSPARSESHARED_EXPORT Dictionary {
private:
    std::vector<DEntry*> elements_;
public:
    Dictionary();
    DEntry* getFirst();
    DEntry* getLast();

};

#endif // DICTIONARY_H
