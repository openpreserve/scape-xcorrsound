#ifndef WAVSTDINREADER_GUARD
#define WAVSTDINREADER_GUARD

#include <cstring>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <vector>

class wavStdinReader {
private:
    char *buf;
    size_t channels;
    size_t bitsPerSample;

    size_t startOfData;
    size_t sizeOfData;
    
    uint32_t sizeOfInput;
public:
    wavStdinReader();
    ~wavStdinReader();
    void getSamplesForChannel(size_t channel, std::vector<int16_t> &samples);
};

#endif
