#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <vector>
#include "libsparse_global.h"
//#include "vigra/matrix.hxx"
#include "samples.h"


#include <eigen2/Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN

#define DICT_VERSION 2

class LIBSPARSESHARED_EXPORT MetaDict {
public:
  int usage_;

};

class LIBSPARSESHARED_EXPORT Dictionary {
private:
    MatrixXd* data_;
    int signalSize_;
    int blockSize_;
    int elements_;
    int channels_;

public:
    std::vector<MetaDict> meta_;

    Dictionary(int, int, int);
    MatrixXd & getData();
    int getSignalSize() {return signalSize_; }
    int getElementCount() {return elements_; }
    int getChannels() {return channels_; }
    int getSize() {return blockSize_; }
    void initRandom();
    void initFromData(Samples& data);

    void normalize();
    void centeR();

    void init(int signalSize, int elements);
    void clear();

    void save(const char* filename);
    void load(const char* filename);


    // debug
    void debugSaveImage(const char* filename);
};

#endif // DICTIONARY_H
