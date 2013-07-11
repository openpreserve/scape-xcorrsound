#ifndef FINGERPRINT_STRATEGY_GUARD
#define FINGERPRINT_STRATEGY_GUARD

#include <stdint.h>
#include <string>
#include <vector>

namespace sound_index {

    class fingerprint_strategy {
    public:

        virtual void getFingerprintsForFile(std::string filename, std::vector<uint32_t> &res) = 0;

        virtual size_t getFrameLength() = 0;

        virtual size_t getAdvance() = 0;
        
        virtual size_t getSampleRate() = 0;
        
    };

    class fingerprint_strategy_ismir : public fingerprint_strategy {
        virtual void getFingerprintsForFile(std::string filename, std::vector<uint32_t> &res);

        virtual size_t getFrameLength();

        virtual size_t getAdvance();
        
        virtual size_t getSampleRate();
    };

    class fingerprint_strategy_chroma : public fingerprint_strategy {
        virtual void getFingerprintsForFile(std::string filename, std::vector<uint32_t> &res);

        virtual size_t getFrameLength();

        virtual size_t getAdvance();
        
        virtual size_t getSampleRate();
    };

}

#endif // FINGERPRINT_STRATEGY_GUARD
