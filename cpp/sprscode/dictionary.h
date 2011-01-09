#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <vector>
#include "sprscode_global.h"
//#include "vigra/matrix.hxx"
#include "samples.h"


#include <eigen2/Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN

#define DICT_VERSION 3

class LIBSPARSESHARED_EXPORT MetaUsage {
public:
    int usage_;
    int id_;
    MetaUsage(int col): usage_(0), id_(col) {};
};

class LIBSPARSESHARED_EXPORT MetaDict {
public:
  std::vector<MetaUsage> col_;
  bool rewrite_;
  long samples_;
  MetaDict():rewrite_(0), samples_(0) {};
};

class LIBSPARSESHARED_EXPORT Dictionary {
private:
    MatrixXd* data_;
    int signalSize_;
    int blockSize_;
    int elements_;
    int channels_;

public:
    MetaDict* meta_;

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

    void sort();
    void save(const char* filename);
    void load(const char* filename);


    // debug
    void debugSaveImage(const char* filename);
};

#endif // DICTIONARY_H
