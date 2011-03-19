#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "sprscode.h"
#include "math.h"
#include "huffman.h"
#include "rle.h"
#include "sprscode.h"

Sprscode::Sprscode(int width, int height, int depth, int blockSize, int coeffsNum)
{
    header_.width_ = width;
    header_.height_ = height;
    header_.depth_ = depth;
    header_.blockSize_ = blockSize;
    header_.quant_ = 0;
    header_.coeffs_ = coeffsNum;
    header_.count_ = 0;

    init();

}


void Sprscode::init() {
    shiftNum_ = (ceil((float)header_.width_/(float)header_.blockSize_) * ceil((float)header_.height_/(float)header_.blockSize_));
    indicesNum_ = shiftNum_*header_.coeffs_;
    coeffsNum_ = indicesNum_;
    shift_ = new char[shiftNum_];
    indices_ = new unsigned short[indicesNum_];
    coeffs_ = new char[coeffsNum_];
}


void Sprscode::load(std::string& fileName) {
    FILE * pFile;
    pFile = fopen(fileName.c_str(),"r");

    fread(&header_,sizeof(SprsHeader), 1, pFile);

    init();

    unsigned char* outBuf = (unsigned char*)malloc(sizeof(char)*coeffsNum_*2);
    int outSize = 0;
    int inSize = 0;

    inSize = shiftNum_*sizeof(char);
    fread(&outSize,sizeof(int),1, pFile);
    fread((char*)outBuf, sizeof(char),outSize, pFile );
//    std::cout <<  inSize << " " << outSize << std::endl;
    Huffman_Uncompress((unsigned char*)outBuf,(unsigned char*)shift_, outSize, inSize);

    inSize = header_.count_*sizeof(unsigned short);
    fread(&outSize,sizeof(int),1, pFile);
    fread((char*)outBuf, sizeof(char),outSize, pFile );
//    std::cout <<  inSize << " " << outSize << std::endl;
    Huffman_Uncompress((unsigned char*)outBuf,(unsigned char*)indices_, outSize, inSize);

    inSize = header_.count_*sizeof(char);
    fread(&outSize,sizeof(int),1, pFile);
    fread((char*)outBuf, sizeof(char),outSize, pFile );
//    std::cout << inSize << " " << outSize << std::endl;
    Huffman_Uncompress((unsigned char*)outBuf,(unsigned char*)coeffs_, outSize, inSize);


    fclose(pFile);
}

void Sprscode::save(std::string& fileName) {

    FILE * pFile;
    pFile = fopen(fileName.c_str(),"w");
    fwrite(&header_,sizeof(SprsHeader), 1, pFile);

    unsigned char* outBuf = (unsigned char*)malloc(sizeof(char)*coeffsNum_*2);

    long fullSize = 0;
    int outSize = 0;
    int inSize = 0;

    inSize = shiftNum_*sizeof(char);
    outSize = Huffman_Compress((unsigned char*)shift_,(unsigned char*)outBuf, inSize);
    fullSize += outSize;
//    std::cout <<  inSize << " " << outSize << std::endl;
    fwrite(&outSize, sizeof(int),1, pFile );
    fwrite((char*)outBuf, sizeof(char),outSize, pFile );

    inSize = header_.count_*sizeof(unsigned short);
    outSize = Huffman_Compress((unsigned char*)indices_,(unsigned char*)outBuf, inSize);
    fullSize += outSize;
//    std::cout <<  inSize << " " << outSize << std::endl;
    fwrite(&outSize, sizeof(int),1, pFile );
    fwrite((char*)outBuf, sizeof(char),outSize, pFile );

    inSize = header_.count_*sizeof(char);
    outSize = Huffman_Compress((unsigned char*)coeffs_,(unsigned char*)outBuf, inSize);
    fullSize += outSize;
//    std::cout <<  inSize << " " << outSize << std::endl;
    fwrite(&outSize, sizeof(int),1, pFile );
    fwrite((char*)outBuf, sizeof(char),outSize, pFile );

    std::cout << "BPP: " <<  ((float)fullSize*8.0)/(float)(header_.height_*header_.width_) << std::endl;
    std::cout << "compression: " <<  (float)(header_.height_*header_.width_*header_.depth_)/((float)fullSize) << ":1" << std::endl;

    fclose(pFile);
}

void Sprscode::uncompress(VectorXd& shift, Eigen::DynamicSparseMatrix<double>& A) {
    unsigned char prev = 127;
    for (int k=0; k<shift.size(); ++k) {
//        shift(k) = shift_[k];

//        std::cout << shift(k) << " " << (int)shift_[k] << " ";
       shift(k) = (double)prev + (double)shift_[k];
       prev = (char)round(shift(k));
//       std::cout << shift(k) << std::endl;
    }

    long lastpos=0;

//    A.startFill();
    for(int i=0; i<header_.count_; ++i) {
        long pos = lastpos + indices_[i];
        lastpos = pos;
        int col = std::ceil(pos / A.rows());
        int row = pos - col*A.rows();
//        std::cout << pos << " " << (double)coeffs_[i] << " " << row << " " << col << " " << indices_[i] << std::endl;
        if(coeffs_[i]) {
//            std::cout <<  A.coeffRef(row,col) << " "  << (double)coeffs_[i] << std::endl;
            A.coeffRef(row,col) = (double)coeffs_[i]*(40.0+(double)row/100.0);
        }
    }
//    A.endFill();
//    for(int col=0; col<A.cols(); ++col) {
//        for(int j=0; j<header_.coeffs_; ++j) {
//            int pos = col*header_.coeffs_+j;
//            int index = indices_[pos];
//            if(coeffs_[pos]) {
//                std::cout << index << " " << col << " " <<  A.coeffRef(index,col) << " "  << (double)coeffs_[pos] << std::endl;
//                A.coeffRef(index,col) = coeffs_[pos]*header_.quant_;
//            }
//        }
//    }
}

void Sprscode::compress(VectorXd& shift, Eigen::SparseMatrix<double>& A) {
    char prev = 127;

    for (int k=0; k<shift.size(); ++k) {
        int shiftVal = round(shift(k));
        shift_[k] = shiftVal-prev;
        prev = shiftVal;
        shift(k) = shiftVal;
    }


//    int count=0;
    long lastpos=0;
    //    for(int k=0; k<A.cols(); ++k) {
    //        for(int j=0; j<A.rows(); ++j) {
    for (int k=0; k<A.outerSize(); ++k) {
        //        std::cout << k << std::endl;
        bool write = false;
        for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {

            int pos=0;
//            pos = (unsigned short)it.row();
            pos = (int)it.col()*A.rows()+it.row();

            double quant = 40.0+(double)it.row()/100.0;
            char data=0;
            data = (char)round(it.value()/quant);
            A.coeffRef(it.row(),it.col()) = (double)data*quant;
//            if(!data) pos = -1;
            if(data) {
                unsigned short delta = pos - lastpos;
//                std::cout << delta << std::endl;
                lastpos = pos;
//                std::cout << delta << std::endl;
                indices_[header_.count_] = delta;
                coeffs_[header_.count_] = data;

                header_.count_++;
                write =true;

            }
        }
        // fill up
        if(!write) {
                int pos = (int)k*A.rows()+A.rows();
                unsigned short delta = pos - lastpos;
                lastpos = pos;
                indices_[header_.count_] = delta;
                coeffs_[header_.count_] = 0;
                header_.count_++;
//            }
        }
    }
    std::cout << "avgCoeffs: " << (double)header_.count_/A.outerSize() << std::endl;
}
