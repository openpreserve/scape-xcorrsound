#ifndef MY_DATABASE_H
#define MY_DATABASE_H

#include <vector>

#include "my_fingerprint.h"

typedef size_t index_t;

class my_database {
private:
    
public:
    std::vector<size_t> retrieveIndices(const my_fingerprint &queryFingerprint);
    void insert(const my_fingerprint &f, const index_t index);
    void insert_file(const char * file);
    std::vector<int16_t> getSamplesFromIndex(index_t index);
    void printStatistics();
    void writeToDisk(const char * file);
    void loadFromDisk(const char * file);
};

#endif
