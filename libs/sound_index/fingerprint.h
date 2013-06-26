#ifndef FINGERPRINTGUARD
#define FINGERPRINTGUARD

#include <vector>
#include <cmath>
#include <complex>
#include "sys/time.h"
#include "stdint.h"

#include <fftw3.h>

static const double PI = 3.14159265;

using std::vector; using std::complex;

template<typename T1, typename T2>
void computeFFT(const vector<T1> &input, vector<complex<T2> > &output) {

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

template<typename T1>
void computePowerSpectrum(vector<T1> &sample, vector<T1> &output) {
    vector<std::complex<double> > tmp(sample.size());
    computeFFT(sample, tmp);
    output.resize(sample.size());
    for (size_t i = 0; i < tmp.size(); ++i) {
	output[i] = tmp[i].real()*tmp[i].real() + tmp[i].imag() * tmp[i].imag();
    }
}

/**
 * Assume we are working with 44100 samples rate
 * and that we hash samples of size 2048.
 * 
 * returns hertz value equal to index entry in the fourier
 * transform, using the above assumptions.
 */
double convertToHertz(size_t index, size_t sampleSize) {
    double sampleRate = 5000.0;

    double T = (0.0+sampleSize) / sampleRate;
	
    return (index+0.0) / T;
}
    
/**
 * Given some frequency, hertz, this function returns
 * the corresponding Mel frequency.
 *
 * Forumla used:
 * m = 2595 log_10 (1+hertz/700)
 * Reference: http://en.wikipedia.org/wiki/Mel_scale
 */
double convertHertzToMel(double hertz) {
    return 2595*log10(1+hertz/700.0);
}

/**
 * Computes the Hann weight (windowing function)
 * Reference: http://en.wikipedia.org/wiki/Window_function#Hann_window
 *
 * @param start Where the Hann window starts
 * @param end One past where the Hann window ends.
 * @param index the value for which you want the Hann weight.
 */
double hannWeight(size_t start, size_t end, size_t index) {
    if (index >= end || index < start) return 0.0;
    size_t length = end-start;
    index = index - start;
    return 0.5 * (1 - cos((2*PI*index)/(length-1)));
}

/**
 * This class represents the fingerprint used in
 * "A Highly Robust Audio Fingerprinting System" by
 * Jaap Haitsma and Ton Kalker (2002, ismir conference).
 */
class Fingerprint {
private:
    std::vector<uint32_t> F; //256 * 4 byte = 1kb
public:

    vector<uint32_t>& getFingerprint() {
	return F;
    }

    Fingerprint(vector<int16_t> &sample) : F(256) {
	
	if (sample.size() != 18368) {
	    // error
	}

	vector<vector<int16_t> > frames(256);
	// The following can surely be done better.
	for (size_t frame = 0; frame < 256; ++frame) {
	    for (size_t j = frame*64; j < frame*64+2048; ++j) {
		frames[frame].push_back(sample[j]);
	    }
	}




	for (size_t f = 0; f < 256; ++f) {

	    F[f] = 0;
	    computePowerSpectrum(frames[f], frames[f]);
	    double prev = 0.0;
	    double melInc = 160.0;
	    size_t prevIdx = 0;
	    for (size_t bit = 0; bit < 33; ++bit) {
//	timeval before, after;
//	gettimeofday(&before, NULL);

		// compute band and sum.
		double curr = 0;
		for (; convertHertzToMel(convertToHertz(prevIdx, 2048)) < (bit+1)*melInc; ++prevIdx) {
		    curr += frames[f][prevIdx];
		}
//	gettimeofday(&after, NULL);
//	uint64_t timeSpent = after.tv_sec * 1000000 + after.tv_usec - (before.tv_sec*1000000 + before.tv_usec);
//	std::cout << "time : " << timeSpent << std::endl;
		
		if (bit > 0) {
		    double res = prev - curr;
		    if (res > 0) {
			uint32_t bitmask = (1<<(bit-1));
			F[f] = F[f] | bitmask;
		    }
		}
		prev = curr;
	    }

	}
    }
/*
    Fingerprint(vector<int16_t> &sample) : F(256) {
	// Use mel-scale to create logarithmic spacing.
	// 5000hz ~= 2360 mel
	// dividing into 33 bands between 0~5000
	// #frames = 256

	vector<vector<double> >  E(257, vector<double>(33, 0.0));
	
	//                               <start position of last frame>
	// sample.size() = frameLength + overlapFactor*frameLength*256 = frameLength(1+overlapFactor*256)
	// => frameLength = sample.size() / (1+overlapFactor*256)
	double overlapFactor = 31.0/32.0;
	double frameLength = (0.0+sample.size()) / (1+overlapFactor*256);
	double overlapLength = overlapFactor * frameLength;


	for (size_t i = 0; i < 256; ++i) {
	    size_t start = overlapLength * i;
	    size_t end = overlapLength * i + frameLength;
	    if (end >= sample.size()) {
		std::cout << "something's up..." << std::endl;
		end = sample.size()-1;
	    }

	    vector<int16_t> input(sample.begin()+start, sample.begin()+end);
	    vector<std::complex<double> > output(input.size(), 0.0);
	    computePowerSpectrum(input, output);

	    double melIncrease = 2300.0/33.0;
	    double melLo = 0;
	    size_t loIdx = 0;
	    for (size_t j = 0; j < 33; ++j) {

		double melHi = melLo + melIncrease;
		
		double hz = convertToHertz(loIdx, output.size());
		double mel = convertHertzToMel(hz);
		//std::cout << "outputSize: " << output.size() << " hz: " << hz << " mel: " << mel << std::endl;
		size_t hiIdx = loIdx;
		for (size_t k = loIdx; melHi > mel; ++k) {
		    ++hiIdx;
		    mel = convertHertzToMel(convertToHertz(k, output.size()));
		}
		melLo = melHi;
		double avg = 0.0;
		size_t items = 0;
		for (size_t k = loIdx; k < hiIdx; ++k) {
		    avg += output[k].real() * hannWeight(loIdx, hiIdx, k);
		    ++items;
		}


		avg = avg+0.0 / items;
		E[i][j] = avg; //std::cout << "E[" << i << "][" << j << "] = " << E[i][j] << std::endl;
		
		loIdx = hiIdx;
	    }

	}

	for (size_t i = 1; i <= 256; ++i) {
	    for (size_t j = 0; j < 32; ++j) {
		double sum = E[i][j]-E[i][j+1] - (E[i-1][j] - E[i-1][j+1]);
		if (sum > 0) {
		    uint32_t temp = 1;
		    temp = temp << j;
		    F[i-1] = F[i-1] | temp;
		} else {
		    uint32_t temp = 1;
		    temp = temp << j;
		    temp = ~temp;
		    F[i-1] = F[i-1] & temp;
		}
	    }
	}

    }
*/
};

struct bitstring {

    size_t size;
    uint32_t *array;

    ~bitstring() {
	delete[] array;
    }

    bitstring() : size(96) {
	array = new uint32_t[96/32];
    }

    bitstring(size_t n) : size(n) {
	array = new uint32_t[(n-1)/32+1];
    }

    void set(size_t i, int val) {
	if (i > size) return;
	val = val & 1;
	size_t idx = i/32;
	if (val) {
	    array[idx] = array[idx] | (1<<(i%32));
	} else {
	    array[idx] = array[idx] & ~(1<<(i%32));
	}
    }

    uint32_t get(size_t i) {
	if (i >= size) return 0;
	return (array[i/32] >> (i%32)) & 1;
    }
};

struct hash96bit {
    bitstring *hashString;
   
    hash96bit(vector<complex<double> >::iterator begin, vector<complex<double> >::iterator end) : 
	hashString(new bitstring(96)) {

	// assume freq. for transform.size() is 44100
	size_t n = end-begin;
	vector<double> temp(2*n/5, 0.0); // cut off down to ~17.5khz
	// overlay a 32x8 grid (32 wide, 8 high)

	for (size_t i = 0; i < temp.size(); ++i) {
	    complex<double> val = *(begin+i);
	    temp[i] = 10*log((val.real() * val.real() + val.imag()*val.imag()));
	    //temp[i] /= log(2);
	}
	
	size_t mult = temp.size()/32;

	for (size_t i = 0; i < 32; ++i) {
	    double max = -10000.0;
	    double avg = 0.0;
	    for (size_t j = i*mult; j < temp.size() && j < (i+1)*mult; ++j) {
		if (max < temp[j]) max = temp[j];
		avg += temp[j];
	    }
	    avg /= mult;
	    uint32_t rounded = static_cast<uint32_t>(max)/36.0; // magic constant.
	    rounded = rounded & 7;

	    hashString->set(3*i, rounded & 1);
	    hashString->set(3*i+1, (rounded & 2)>>1);
	    hashString->set(3*i+2, (rounded & 4)>>2);
	}
    }

    ~hash96bit() {
	delete hashString;
    }
};

#endif //FINGERPRINTGUARD
