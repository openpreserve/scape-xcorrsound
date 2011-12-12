#include "AudioFile.h"

#include <cstdio>
#include <vector>
#include <stdint.h>
#include <iostream>
#include "my_utils.h"

typedef long long ll;

using namespace std;

AudioFile::AudioFile(const char *path) : fd(fopen(path,"r")), _channels(0), _sampleRate(0),
					 _samplesPrChannel(0), _fileSize(0), _audioFormat(0),
					 _byteRate(0), _blockAlign(0), _bitsPrSample(0), _filename(path) {
    populateFieldVariables();
}

AudioFile::~AudioFile() {
    fclose(fd);
}

size_t AudioFile::getNumberOfChannels() {
    if (_channels) return _channels;
    populateFieldVariables();
    return _channels;
}

uint32_t AudioFile::getSampleRate() {
    return this->_sampleRate;
}

size_t AudioFile::getNumberOfSamplesPrChannel() {
    return this->_samplesPrChannel;
}

void AudioFile::getSamplesForChannel(size_t channel, std::vector<short> &out) {
    // do something.
    fseek(fd, 0, SEEK_END);
    fseek(fd, 44, SEEK_SET);

    uint8_t buf[2048];
    size_t currPos = 44;
    size_t read = 0;
    while (!feof(fd)) {
	fseek(fd, read, SEEK_CUR);
	size_t read = fread(buf, 1, sizeof(buf), fd);	
	for (size_t i = 0; i < read; i+=4) {
	    out.push_back(getIntFromChars(buf[i], buf[i+1]));
	}
	currPos += read;
    }
}

void AudioFile::populateFieldVariables() {
    ll currentPos = ftell(fd);

    fseek(fd, 0, SEEK_END);
    this->_fileSize = ftell(fd);
    if (!(this->_fileSize)) return;

    uint8_t buf[44];
    fseek(fd, 0, SEEK_SET);
    size_t read = fread(buf, 1, 44, fd);
    if (read < 44) return; // error..
    fseek(fd,currentPos, SEEK_SET);
    // populate field variables now...

    this->_audioFormat = getIntFromChars(buf[20], buf[21]);
    this->_channels = getIntFromChars(buf[22], buf[23]);
    this->_sampleRate = getIntFromChars(buf[24], buf[25], buf[26], buf[27]);
    this->_samplesPrChannel = (this->_fileSize - 44)/this->_channels/2; //2 bytes pr sample.
}

AudioStream AudioFile::getStream(size_t channel) {
    return AudioStream(channel, this->_channels, this->_filename);
}
