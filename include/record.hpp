#pragma once
#include <fstream>

const int val_sz=64;
const int key_sz=32;

struct Record{
    char key[key_sz];
    char val[val_sz];
    bool alive; //tombstone flag
};

void rec_write(std::fstream &fo, const Record &r);
Record rec_read(std::fstream &fo, std::streampos offset);
