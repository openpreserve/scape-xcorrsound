#include <vector>
#include <iostream>
#include <complex>
#include <cmath>
#include <algorithm>

#include <stdint.h>

#include "computeFFT.h"
#include "../AudioFile.h"

using namespace std;

size_t sampleLength = 2048;
size_t sampleRate = 5512;
size_t jump = 150;

vector<double> points[12];

static double getHz(const size_t idx, const size_t length = 4096, const size_t sampleRate = 8192) {
	
    double T = length / static_cast<double>(sampleRate);
    return idx / T;
}

static double getMel(const double hz) {
    return 2595 * std::log10(1+hz/700.0);
}


int main(int argc, char* argv[]) {

    for (int i = 1; i < argc; ++i) {
	AudioFile a(argv[i]);
	vector<int16_t> samples;
	a.getSamplesForChannel(0, samples);

	for (size_t j = sampleLength; j < samples.size(); j+= jump) {

	    vector<float> chunk(samples.begin() + j - sampleLength, samples.begin() + j);
	    // normalize chunk
	    float maximum = 1.0;
	    for (size_t k = 0; k < chunk.size(); ++k) {
		if (chunk[k] < 0 && -chunk[k] >= maximum) maximum = -chunk[k];
		else if (chunk[k] > maximum) maximum = chunk[k];
	    }
	    for (size_t k = 0; k < chunk.size(); ++k) {
		chunk[k] = chunk[k] / maximum;
	    }

	    vector<complex<float> > transform(chunk.size());
	    computeFFT(chunk, transform);

	    double melIncrease = 181.67;

	    size_t k = 0;
	    for (int l = 0; l < 12; ++l) {
		double currentMel = 220.0 + (l+1) * melIncrease;
		double avg = 0.0;
		size_t items = 0;

		for (; getMel(getHz(k, sampleLength, sampleRate)) < currentMel ; ++k) {
		    ++items;
		    avg += transform[k].imag() * transform[k].imag() + transform[k].real()*transform[k].real();
		}

		avg /= items;
		
		points[l].push_back(avg);
		
	    }
	}
	cout << "fisk" << endl;
    }


    // output fingerprint function.
    cout << "buckets[12][7] = {";
    for (size_t i = 0; i < 12; ++i) {
	cout << "{";
	sort(points[i].begin(), points[i].end());

	for (size_t j = 1; j < 8; ++j) {
	    size_t pos = (j*points[i].size()) / 8;
	    double val = (points[i][pos] + points[i][pos-1]) / 2;
	    
	    cout << val;
	    if (j != 7) cout << ", ";
	}
	cout << "}";
	if (i != 11) cout << "," << endl;

    }
    cout << "}" << endl;
    


}
