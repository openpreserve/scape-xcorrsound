#ifndef MY_FINGERPRINT_H
#define MY_FINGERPRINT_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

#include <stdlib.h>

#include <stdint.h>

#include <fftw3.h>

#include "computeFFT.h"

class my_fingerprint {
private:

    uint64_t fingerprint;
    
    // time in seconds.
    inline
    static double getHz(const size_t idx, const size_t length = 4096, const size_t sampleRate = 8192) {
	
	double T = length / static_cast<double>(sampleRate);
	return idx / T;
    }

    inline
    static double getMel(const double hz) {
	return 2595 * std::log10(1+hz/700.0);
    }

public:

    my_fingerprint(std::vector<int16_t>::iterator begin, std::vector<int16_t>::iterator end, bool sorting = false);
    my_fingerprint(uint64_t fp) : fingerprint(fp) {};
    uint64_t getPrint() const;

    bool operator<(const my_fingerprint &other) const;
};

#endif
