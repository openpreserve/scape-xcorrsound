#include <iostream>
#include <vector>
#include <complex>

#include "stdint.h"

#include "fingerprint.h"
#include "../AudioFile.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;
    AudioFile af(argv[1]);

    vector<short> samples;
    af.getSamplesForChannel(0, samples);


    for (size_t i = 0; i < 10; ++i) {
	vector<complex<double> > transformed;
	vector<short> input(samples.begin()+i*20, samples.begin()+1024+i*20);
	computeFFT(input, transformed);
	hash96bit hash(transformed.begin(), transformed.end());
	for (size_t j = 0; j < 96; ++j) {
	    cout << hash.bs.get(j);
	}
	cout << endl;
    }

}
