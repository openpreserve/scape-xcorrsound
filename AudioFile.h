#ifndef AUDIO_FILE_GUARD
#define AUDIO_FILE_GUARD

#include <cstdio>
#include <vector>

class AudioFile {
private:
    std::FILE *fd;
    size_t _channels;
    size_t _sampleRate;
    size_t _samplesPrChannel;
    size_t _fileSize;
    size_t _audioFormat;
    size_t _byteRate;
    size_t _blockAlign;
    size_t _bitsPrSample;

    void populateFieldVariables();
public:
    AudioFile(const char *path);
    ~AudioFile();

    size_t getNumberOfChannels();
    size_t getSampleRate();
    size_t getNumberOfSamplesPrChannel();
    void getSamplesForChannel(size_t channel, std::vector<short>&);

};

#endif
