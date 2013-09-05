#include <cstring>
#include <iostream>
#include <my_utils.h>
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <wavStdinReader.hh>

wavStdinReader::wavStdinReader() {
    char riff[4];
    
    size_t bytesRead = read(0, &riff[0], 4);
    if (bytesRead != 4 || strncmp(riff, "riff", 4) != 0) {
        // error: wrong format.
    }

    bytesRead = read(0, &sizeOfInput, 4);
    if (bytesRead != 4) {
        // error: not enough input.
    }

    buf = new char[sizeOfInput];
    bytesRead = read(0, buf, sizeOfInput);
    while (bytesRead != sizeOfInput) {
        bytesRead += read(0, &buf[bytesRead], sizeOfInput-bytesRead);
    }
    // if (bytesRead != sizeOfData) {
    //     // error: header and filesize do not correspond.
    // }

    char dataStr[] = "data";
    this->startOfData = 0;
    for (size_t i = 0; i < sizeOfInput-4; ++i) {
        if (strncmp(dataStr, &buf[i], 4) == 0) {
            this->startOfData = i+8;
            break;
        }
    }
    sizeOfData = buf[startOfData-4] + (((int)buf[startOfData-3])<<8) +
        (((int)buf[startOfData-2]) << 16) + (((int)buf[startOfData-1]) << 24);
    
    sizeOfData = sizeOfInput-startOfData;

    if (strncmp(&buf[0], "fmt ", 2) != 0) {
        // error: wrong format.
    }

    if (!(buf[12] == 1 && buf[13] == 0)) {
        // error: we only support PCM.
    }

    channels = buf[14] + (((int)buf[15])<<8);

    if (!(buf[16] == -120 && buf[17] == 21 && buf[18] == 0 && buf[19] == 0)) {
        // error: we only support 5512hz samplerate.
    }

    if (!(buf[26] == 16 && buf[27] == 0)) {
        // error: we only support 16 bits/sample
    }
    
}

wavStdinReader::~wavStdinReader() {
    delete[] buf;
}

void
wavStdinReader::getSamplesForChannel(size_t channel, std::vector<int16_t> &samples) {
    if (channel > this->channels) {
        // error
        return;
    }

    samples.resize(sizeOfData / channels / 2);

    size_t numSamples = sizeOfData / channels / 2;

    for (size_t i = 0; i < numSamples; ++i) {
        size_t pos = startOfData + i*channels*2 + channel*2;
        samples[i] = convertTwoBytesToShort(buf[pos], buf[pos+1]);
        // samples[i] = ((buf[pos+1] & 255) << 8);
        // samples[i] = samples[i] | (buf[pos] & 255);
    }

    return;
}
