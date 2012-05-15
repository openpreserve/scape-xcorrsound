#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <string>
#include <vector>
#include <cstdio>
#include "stdint.h"

#include "my_utils.h"

class AudioStream {
private:
    const static size_t _bufferSize = 4096;
    char _buf[_bufferSize];

    FILE *_file;
    size_t _channel; // 0 is left, 1 is right.
    size_t _channels;
    size_t _fileSize;
    size_t _pos;
    size_t _end;


    inline void fillBuffer() {
	_end = fread(_buf, 1, _bufferSize, _file);
	_pos = 0;
    }
    
    inline uint8_t readNext() {
	if (_pos == _end) {
	    fillBuffer();
	}
	return _buf[_pos++];
    }

    inline bool good() {
	if (ferror(_file) != 0) return false;
	if (_pos == _end && feof(_file) != 0) return false;

	return true;
    }

public:
    
    AudioStream(size_t channel, size_t channels, std::string filename, size_t startOfData) 
	: _file(fopen(filename.c_str(), "r")),
	  _channel(channel),
	  _channels(channels),
	  _pos(0),
	  _end(0) {	

	fseek(_file, startOfData, SEEK_SET);
    }

    ~AudioStream() {
	fclose(_file);
    }
    
    /**
     * invariant: after each read we are at the next sample
     */
    void read(size_t samples, std::vector<short> &res) {
	res.resize(samples);
	size_t i;
	for (i = 0; i < samples && !feof(_file); ++i) {
	    if (!good()) {
		break;
	    }

	    for (size_t j = 0; j < _channel; ++j) {
		readNext();
		readNext();
	    }

	    char first = readNext();
	    char second = readNext();
	    for (size_t j = _channel+1; j < _channels; ++j) {
		readNext(); // resetting to keep invariant.
		readNext();
	    }
	    res[i] = convertTwoBytesToShort(first, second);
	}
	res.resize(i);
    }

};
#endif // AUDIO_STREAM_H
