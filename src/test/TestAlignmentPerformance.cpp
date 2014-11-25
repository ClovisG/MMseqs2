//  main.cpp
//  forautocompl
//
//  Created by Martin Steinegger on 26.11.12.
//  Copyright (c) 2012 -. All rights reserved.
//
#include <iostream>
#include "Sequence.h"
#include "ExtendedSubstitutionMatrix.h"
#include "SubstitutionMatrix.h"
#include "../alignment/smith_waterman_sse2.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "kseq.h"
#define MAX_FILENAME_LIST_FILES 4096

KSEQ_INIT(int, read);


std::vector<std::string> readData(std::string fasta_filename){
    std::vector<std::string> retVec;
    kseq_t *seq;
    FILE* fasta_file = fopen(fasta_filename.c_str(), "r");
    if(fasta_file == NULL) {std::cout << "Could not open " << fasta_filename<<std::endl; EXIT(1); }
    seq = kseq_init(fileno(fasta_file));
    int l;
    size_t entries_num = 0;
    while ((l = kseq_read(seq)) >= 0) {
        if (entries_num > 1000)
            break;
        if (seq->seq.l > 500) {
            std::string sequence = seq->seq.s;
            retVec.push_back(sequence);
            entries_num++;
        }
    }
    return retVec;
}
int main (int argc, const char * argv[])
{

    const size_t kmer_size=6;

    SubstitutionMatrix subMat("/Users/mad/Documents/workspace/mmseqs/data/blosum62.out",2.0);
    std::cout << "Subustitution matrix:\n";
    SubstitutionMatrix::print(subMat.subMatrix,subMat.int2aa,subMat.alphabetSize);
    std::cout << "\n";

    std::cout << "subMatrix:\n";
    std::cout << "\n";


    std::cout << "Sequence (id 0):\n";
    Sequence* query = new Sequence (10000, subMat.aa2int, subMat.int2aa, 0, kmer_size, true);
    Sequence* dbSeq = new Sequence(10000,subMat.aa2int, subMat.int2aa, 0, kmer_size, true);
    //dbSeq->mapSequence(1,"lala2",ref_seq);
    SmithWaterman aligner(15000, subMat.alphabetSize);
    int8_t * tinySubMat = new int8_t[subMat.alphabetSize*subMat.alphabetSize];
    for (size_t i = 0; i < subMat.alphabetSize; i++) {
        for (size_t j = 0; j < subMat.alphabetSize; j++) {
            std::cout << ( i*subMat.alphabetSize + j) << " " << subMat.subMatrix[i][j] << " ";

            tinySubMat[i*subMat.alphabetSize + j] = (int8_t)subMat.subMatrix[i][j];
        }
        std::cout << std::endl;
    }

    int gap_open = 10;
    int gap_extend = 1;
    int mode = 0;
    size_t cells = 0;
    std::vector<std::string> sequences = readData("/Users/mad/Documents/databases/rfam/Rfam.fasta");
    for(int seq_i = 0; seq_i < sequences.size(); seq_i++){
        query->mapSequence(1,"lala2",sequences[seq_i].c_str());
        aligner.ssw_init(query, tinySubMat, subMat.alphabetSize, 2);

        for(int seq_j = 0; seq_j < sequences.size(); seq_j++) {
            dbSeq->mapSequence(2, "WTF", sequences[seq_j].c_str());
            int32_t maskLen = query->L / 2;
            s_align *alignment = aligner.ssw_align(dbSeq->int_sequence, dbSeq->L, gap_open, gap_extend, mode, 0, 0, maskLen);
            if(mode == 0 ){
                cells += query->L * dbSeq->L;
                std::cout << alignment->qEndPos1 << " " << alignment->dbEndPos1 << "\n";
            } else {
                std::cout << alignment->qStartPos1 << " " << alignment->qEndPos1 << " "
                        << alignment->dbStartPos1 << " " << alignment->dbEndPos1 << "\n";
            }
        }
    }
    std::cerr << "Cells : " << cells << std::endl;
    delete [] tinySubMat;
    delete query;
    delete dbSeq;
    return 0;
}
