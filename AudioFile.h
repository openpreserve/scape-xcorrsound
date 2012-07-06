#ifndef AUDIO_FILE_GUARD
#define AUDIO_FILE_GUARD

#include <vector>
#include <cstdio>
#include "stdint.h"
#include <string>
#include "AudioStream.h"

class AudioFile {
private:
    FILE *fd;
    size_t _channels;
    size_t _sampleRate;
    size_t _samplesPrChannel;
    size_t _fileSize;
    size_t _audioFormat;
    size_t _byteRate;
    size_t _blockAlign;
    size_t _bitsPrSample;
    size_t _startOfData;
    std::string _filename;
    void populateFieldVariables();
public:
    AudioFile(const char *path);
    ~AudioFile();

    // end is one past the last sample.
    void getSamplesForChannelInRange(size_t begin, size_t end, std::vector<int16_t>&);
    size_t getNumberOfChannels();
    uint32_t getSampleRate();
    size_t getNumberOfSamplesPrChannel();
    void getSamplesForChannel(size_t channel, std::vector<int16_t>&);
    AudioStream getStream(size_t channel);
};

#endif
