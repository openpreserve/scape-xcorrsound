#include "my_fingerprint.h"

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <iostream>

#include <stdlib.h>

#include "stdint.h"

#include <fftw3.h>

#include "computeFFT.h"

using namespace std;

typedef vector<int16_t>::iterator it;
std::vector<double> blah;

size_t sampleRate = 5512;
size_t sampleLength = 2048;

my_fingerprint::my_fingerprint(it begin, it end, bool sorting) {
    fingerprint = 0;
    double buckets[2][7] = {{4.82e07, 1.12e08, 6.57e08, 2.97e09, 6.53e09, 1.19e10, 2.39e10},
			    {1,2,3,4,5,6,7}};

    vector<int16_t> samples(begin, end);
    vector<complex<double> > transform(samples.size());

    computeFFT(samples, transform);
    
    double melIncrease = 200.0;
    double currentMel = 0;
    size_t i = 0;

    while (currentMel < 2400) {
	double avg = 0.0;
	size_t items = 0;
	currentMel += melIncrease;

	for (; getMel(getHz(i, sampleLength, sampleRate)) < currentMel ; ++i) {
	    ++items;
	    avg += transform[i].imag() * transform[i].imag() + transform[i].real()*transform[i].real();
	}

	avg /= items;
	if (currentMel == 200.0) blah.push_back(avg);
	//std::cout << avg << std::endl;
	uint64_t bucket = 0;
	for (size_t j = 0; j < 7; ++j) {
	    if (avg > buckets[0][j]) ++bucket;
	}

	size_t slab = max(static_cast<size_t>((currentMel/200)-1), static_cast<size_t>(0));
	    
	size_t bitIdxLo = 3*slab;
	bucket = bucket << bitIdxLo;
	    
	fingerprint = fingerprint | bucket;
	    
    }
    if (sorting) {
	sort(blah.begin(), blah.end());
	std::cout << blah[blah.size()/2] << std::endl;
	std::cout << "------------------" << std::endl;
    }
}

uint64_t my_fingerprint::getPrint() const {
    return fingerprint;
}

bool my_fingerprint::operator<(const my_fingerprint &lhs) const {
    return fingerprint < lhs.fingerprint;
}
