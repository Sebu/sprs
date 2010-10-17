#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <vector>
#include "libsparse_global.h"
#include "vigra/matrix.hxx"
#include "samples.h"

class LIBSPARSESHARED_EXPORT Dictionary {
private:
    vigra::Matrix<double>* data_;
    int signalSize_;
    int elementCount_;
    int channels_;
    int size_;
public:
    Dictionary(int, int, int);
    vigra::Matrix<double> & getData();
    int getSignalSize() {return signalSize_; }
    int getElementCount() {return elementCount_; }
    int getChannels() {return channels_; }
    int getSize() {return size_; }
    void initRandom();
    void initFromData(Samples& data);


    void save(const char* filename);
    void load(const char* filename);


    // debug
    void debugSaveImage(char* filename);
};

#endif // DICTIONARY_H
