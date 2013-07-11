#include <fingerprint_strategy.hh>
#include <stdint.h>
#include <string>
#include <vector>

namespace {
    size_t frameLength = 4096;

    size_t advance = 1365;

    size_t sampleRate  = 11025;
}

namespace sound_index {


    void
    fingerprint_strategy_chroma::getFingerprintsForFile(std::string filename, std::vector<uint32_t> &res) {
        // implement chroma.
    }
        
    size_t
    fingerprint_strategy_chroma::getFrameLength() { return ::frameLength; }

    size_t
    fingerprint_strategy_chroma::getAdvance() { return ::advance; }

    size_t
    fingerprint_strategy_chroma::getSampleRate() { return ::sampleRate; }

}
