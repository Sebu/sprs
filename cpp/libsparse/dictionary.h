#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <vector>
#include "libsparse_global.h"
//#include "vigra/matrix.hxx"
#include "samples.h"


#include <eigen2/Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN


class LIBSPARSESHARED_EXPORT Dictionary {
private:
    MatrixXd* data_;
    int signalSize_;
    int elementCount_;
    int channels_;
    int size_;
public:
    Dictionary(int, int, int);
    MatrixXd & getData();
    int getSignalSize() {return signalSize_; }
    int getElementCount() {return elementCount_; }
    int getChannels() {return channels_; }
    int getSize() {return size_; }
    void initRandom();
    void initFromData(Samples& data);

    void normalize();

    void save(const char* filename);
    void load(const char* filename);


    // debug
    void debugSaveImage(const char* filename);
};

#endif // DICTIONARY_H
