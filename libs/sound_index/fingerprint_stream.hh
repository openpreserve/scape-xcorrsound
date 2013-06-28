#ifndef FINGERPRINT_STREAM_GUARD
#define FINGERPRINT_STREAM_GUARD

#include <stdint.h>
#include <vector>

namespace sound_index {

    const size_t frameLength = 2048;

    const size_t advance = 128;

    const size_t sampleRate = 5512;

}

void generateFingerprintStream(std::vector<int16_t> &input, std::vector<uint32_t> &output);

#endif
