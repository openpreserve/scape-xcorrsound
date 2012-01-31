#ifndef FINGERPRINTGUARD
#define FINGERPRINTGUARD

#include <vector>
#include <cmath>
#include <complex>

#include "stdint.h"

#include <fftw3.h>

using namespace std;

template<typename T1, typename T2>
void computeFFT(vector<T1> &input, vector<complex<T2> > &output) {

    fftw_plan plan;
    fftw_complex *t = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*input.size());
    fftw_complex *o = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*input.size());

    for (size_t i = 0; i < input.size(); ++i) {
	t[i][0] = input[i];
	t[i][1] = 0;
    }

    plan = fftw_plan_dft_1d(input.size(), t, o, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_free(t);
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
	output[i] = std::complex<T2>(o[i][0], o[i][1]);
    }
    fftw_free(o);
}

struct bitstring {

    size_t size;
    uint32_t *array;

    bitstring() : size(3) {
	array = new uint32_t[3];
    }

    bitstring(size_t n) : size(n) {
	array = new uint32_t[(n-1)/32+1];
    }

    void set(size_t i, int val) {
	if (i >= size*32) return;
	val = val & 1;
	size_t idx = (i-1)/32 + 1;
	if (val) {
	    array[idx] = array[idx] | (1<<(i%32));
	} else {
	    array[idx] = array[idx] & ~(1<<(i%32));
	}
    }

    uint32_t get(size_t i) {
	return array[(i-1)/32+1] >> (i%32);
    }
};

struct hash96bit {
    bitstring bs;
    
    hash96bit(vector<complex<double> >::iterator begin, vector<complex<double> >::iterator end) {
	//assume freq. for transform.size() is 44100
	size_t n = end-begin;
	vector<double> temp(n/2, 0.0);
	//overlay a 32x8 grid (32 wide, 8 high)

	for (size_t i = 0; i < temp.size(); ++i) {
	    complex<double> val = *(begin+i);
	    temp[i] = log(val.real() * val.real() + val.imag()*val.imag());
	    temp[i] /= log(2);
	}
	
	size_t mult = n/32/2;

	for (size_t i = 0; i < 32; ++i) {
	    double max = -10000.0;
	    for (size_t j = i*mult; j < n/2 && j < (i+1)*mult; ++j) {
		if (max < temp[j]) max = temp[j];
	    }
	    //range is 0-63: divide by 8 and round down.
	    uint32_t rounded = static_cast<uint32_t>(max)/8;
	    bs.set(3*i, rounded & 1);
	    bs.set(3*i+1, rounded & 2);
	    bs.set(3*i+2, rounded & 4);
	}
    }
};

#endif //FINGERPRINTGUARD
