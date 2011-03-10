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

    shiftNum_ = (ceil((float)width/(float)blockSize) * ceil((float)height/(float)blockSize));
    indicesNum_ = coeffsNum_ = shiftNum_*coeffsNum;


    shift_ = new unsigned char[shiftNum_];
    indices_ = new unsigned short[indicesNum_];
    coeffs_ = new char[coeffsNum_];


}

void Sprscode::load(std::string& fileName) {
    FILE * pFile;
    pFile = fopen((fileName + ".sprs").c_str(),"r");

    fread(&header_,sizeof(SprsHeader), 1, pFile);

    unsigned char* outBuf = (unsigned char*)malloc(sizeof(char)*coeffsNum_*2);
    unsigned char* outBuf2 = (unsigned char*)malloc(sizeof(char)*coeffsNum_*2);

    int outSize = 0;
    int inSize = 0;

    inSize = shiftNum_*sizeof(char);
    fread(&outSize,sizeof(int),1, pFile);
    fread((char*)outBuf, sizeof(char),outSize, pFile );
    std::cout << outSize << std::endl;
    Huffman_Uncompress((unsigned char*)outBuf,(unsigned char*)shift_, outSize, inSize);

    inSize = indicesNum_*sizeof(unsigned short);
    fread(&outSize,sizeof(int),1, pFile);
    fread((char*)outBuf, sizeof(char),outSize, pFile );
    std::cout << outSize << std::endl;
    RLE_Uncompress((unsigned char*)outBuf,(unsigned char*)indices_, outSize);// inSize);

    int rleSize = 0;
    inSize = coeffsNum_*sizeof(char);
    fread(&rleSize,sizeof(int),1, pFile);
    fread(&outSize,sizeof(int),1, pFile);
    fread((char*)outBuf, sizeof(char),outSize, pFile );
    std::cout << outSize << std::endl;
    Huffman_Uncompress((unsigned char*)outBuf,(unsigned char*)outBuf2, outSize, rleSize);
    RLE_Uncompress((unsigned char*)outBuf2,(unsigned char*)coeffs_, rleSize);


    fclose(pFile);
}

void Sprscode::save(std::string& fileName) {
    FILE * pFile;
    pFile = fopen((fileName + ".sprs").c_str(),"w");
    fwrite(&header_,sizeof(SprsHeader), 1, pFile);

    unsigned char* outBuf = (unsigned char*)malloc(sizeof(char)*coeffsNum_*2);
    unsigned char* outBuf2 = (unsigned char*)malloc(sizeof(char)*coeffsNum_*2);
    int fullSize = 0;
    int outSize = 0;
    int inSize = 0;

    inSize = shiftNum_*sizeof(char);
    outSize = Huffman_Compress((unsigned char*)shift_,(unsigned char*)outBuf, inSize);
    fullSize += outSize;
    std::cout << outSize << std::endl;
    fwrite(&outSize, sizeof(int),1, pFile );
    fwrite((char*)outBuf, sizeof(char),outSize, pFile );

    inSize = indicesNum_*sizeof(unsigned short);
    outSize = RLE_Compress((unsigned char*)indices_,(unsigned char*)outBuf, inSize);
    fullSize += outSize;
    std::cout << outSize << std::endl;
    fwrite(&outSize, sizeof(int),1, pFile );
    fwrite((char*)outBuf, sizeof(char),outSize, pFile );

    inSize = coeffsNum_*sizeof(char);
    outSize = RLE_Compress((unsigned char*)coeffs_,(unsigned char*)outBuf2, inSize);
    fwrite(&outSize, sizeof(int),1, pFile );
    outSize = Huffman_Compress((unsigned char*)outBuf2,(unsigned char*)outBuf, inSize);
    fullSize += outSize;
    std::cout << outSize <<   std::endl;
    fwrite(&outSize, sizeof(int),1, pFile );
    fwrite((char*)outBuf, sizeof(char),outSize, pFile );


    std::cout << "BPP: " <<  ((float)fullSize*8.0)/(float)(header_.height_*header_.width_) << std::endl;
    std::cout << "compression: " <<  (float)(header_.height_*header_.width_*3)/((float)fullSize) << ":1" << std::endl;

    fclose(pFile);
}

void Sprscode::uncompress(VectorXd& shift, Eigen::SparseMatrix<double>& A) {

    for (int k=0; k<shift.size(); ++k) {
        shift(k) = shift_[k];
        //        shift(k) = prev = shift_[k]-prev;
    }

    for(int col=0; col<A.cols(); ++col) {
        for(int j=0; j<header_.coeffs_; ++j) {
            int pos = col*header_.coeffs_+j;
            int index = indices_[pos];
            //            std::cout << index << " " << col << std::endl;
            if(coeffs_[pos])
                A.coeffRef(index,col) = coeffs_[pos]*header_.quant_;
        }
    }
}

void Sprscode::compress(VectorXd& shift, Eigen::SparseMatrix<double>& A) {
    short prev = 0;
    for (int k=0; k<shift.size(); ++k) {
        unsigned char shiftVal = (unsigned char)round(shift(k));
        shift_[k] = prev = shiftVal-prev;
        //shift(k) = shiftVal;
    }


    int count=0;
    //    for(int k=0; k<A.cols(); ++k) {
    //        for(int j=0; j<A.rows(); ++j) {
    for (int k=0; k<A.outerSize(); ++k) {
        //        std::cout << k << std::endl;
        bool write = false;
        for (Eigen::SparseMatrix<double>::InnerIterator it(A,k); it; ++it) {

            unsigned short pos=0;
            pos = (unsigned short)it.row();
            double quant = 10.0+(double)pos/40.0;
            int data=0;
            //          data =(char)round(A.coeff(j,k)/quant_);
            data = (int)round(it.value()/quant);
            //            std::cout << it.row() << " " << it.col() << std::endl;
          A.coeffRef(it.row(),it.col()) = (double)data*quant;
//                        if(!data) pos = -1;
            if(data) {
//                std::cout << it.value() << std::endl;

                indices_[count] = pos;
                coeffs_[count] = data;
                //                std::cout<< (int)data << std::endl;
                count++;

            }
            write =true;
        }
        // fill up
        if(!write) {
            for(int i=0; i<header_.coeffs_; ++i) {
                indices_[count] = 0;
                coeffs_[count] = 0;
                count++;
            }
        }
    }
    std::cout << "avgCoeffs: " << (double)count/A.outerSize() << std::endl;
}
