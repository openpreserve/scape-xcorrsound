#ifndef SOUND_MATCH_H
#define SOUND_MATCH_H

#include <vector>
#include <complex>
#include <iostream>
#include <algorithm>
#include "cross_correlation.h"
#include "AudioFile.h"
#include "stdint.h"
#include <iomanip>
#include "my_utils.h"
#include "AudioStream.h"

static const double THRESHHOLD = 0.98;

using std::vector; using std::complex; using std::cout; using std::endl;

void prefixSquareSum(vector<short> &a, vector<int64_t> &res) {
    res.resize(a.size());
    res[0] = a[0] * a[0];
    for (size_t i = 1; i < res.size(); ++i) {
	res[i] = res[i-1] + a[i] * a[i];
    }
}

double computeNormFactor(vector<int64_t> &prefixSquareSmall, vector<int64_t> &prefixSquareLarge, 
			 vector<int64_t>::iterator smallBegin, vector<int64_t>::iterator smallEnd,
			 vector<int64_t>::iterator largeBegin, vector<int64_t>::iterator largeEnd) {
    
    if (smallEnd != prefixSquareSmall.begin()) --smallEnd;

    int64_t smallVal = *smallEnd;

    if (smallBegin != prefixSquareSmall.begin()) {
	--smallBegin;
	smallVal -= *smallBegin;
    }

    if (largeEnd != prefixSquareLarge.begin()) --largeEnd;

    int64_t largeVal = *largeEnd;    

    if (largeBegin != prefixSquareLarge.begin()) {
	--largeBegin;
	largeVal -= *largeBegin;
    }

    return 0.5 * (smallVal + largeVal);

}


template<typename T>
std::ostream& operator<<(std::ostream &os, std::vector<T> &l) {
    for (size_t i = 0; i < l.size(); ++i) {
	os << l[i] << " ";
    }
    return os;
}


template<typename T>
std::ostream& operator<<(std::ostream &os, std::complex<T> &c) {
    os << "(" << c.real() << " , " << c.imag() << "i)";
    return os;
}


void match(AudioFile &needle, AudioFile &haystack, std::vector<size_t> &results) {
    std::vector<short> small; std::vector<short> large;
    std::vector<int64_t> smallPrefixSum; std::vector<int64_t> largePrefixSum;
    needle.getSamplesForChannel(0, small);
    prefixSquareSum(small, smallPrefixSum);

    proxyFFT<short, double> smallFFT(small);
    smallFFT.transform();
    
    size_t largeTotalSize = haystack.getNumberOfSamplesPrChannel();
    vector<int64_t> maxSamplesBegin(largeTotalSize/small.size());
    vector<int64_t> maxSamplesEnd(largeTotalSize/small.size());
    size_t stillToRead = largeTotalSize;

    AudioStream hayStream = haystack.getStream(0);
    size_t pieces = 10;
    for (int j = 0; ; ++j) {
	hayStream.read(pieces*small.size(), large);
	prefixSquareSum(large, largePrefixSum);
	size_t numberOfParts = large.size()/small.size();
	size_t idxAdd = j*pieces;

	std::cout << ((largeTotalSize-stillToRead)+0.0)/largeTotalSize*100 << "%" << std::endl;
	stillToRead -= large.size();

	for (size_t ii = 0; ii < numberOfParts*small.size(); ii += small.size()) {
	    //do stuff..
	    proxyFFT<short, double> largeFFT(large.begin()+ii, large.begin()+ii+small.size());
	    
	    vector<complex<double> > outBegin;
	    vector<complex<double> > outEnd;
	    //std::cout << "TEST1" << std::endl;
	    cross_correlation(largeFFT, smallFFT, outBegin);
	    cross_correlation(smallFFT, largeFFT, outEnd);

	    size_t maxSampleBegin = 0;
	    double maxNormFactorBegin = computeNormFactor(smallPrefixSum, largePrefixSum,
							  smallPrefixSum.begin(), smallPrefixSum.end(),
							  largePrefixSum.begin()+ii, largePrefixSum.begin()+small.size()+ii);

	    for (size_t i = 0 ; i < outBegin.size(); ++i) {
		double normFactor = computeNormFactor(smallPrefixSum, largePrefixSum,
						      smallPrefixSum.begin(), smallPrefixSum.end()-i,
						      largePrefixSum.begin()+i+ii, largePrefixSum.begin()+ii+small.size());
	
		if (outBegin[maxSampleBegin].real()/maxNormFactorBegin < outBegin[i].real()/normFactor) {
		    maxSampleBegin = i;
		    maxNormFactorBegin = normFactor;
		}
	    }
	    //std::cout << "TEST2" << std::endl;
	    size_t maxSampleEnd = 0;
	    double maxNormFactorEnd = computeNormFactor(smallPrefixSum, largePrefixSum,
							smallPrefixSum.begin(), smallPrefixSum.end(),
							largePrefixSum.begin()+ii, largePrefixSum.begin()+small.size()+ii);
	    for (size_t i = 0 ; i < outEnd.size(); ++i) {
		double normFactor = computeNormFactor(smallPrefixSum, largePrefixSum,
						      smallPrefixSum.begin()+i, smallPrefixSum.end(),
						      largePrefixSum.begin()+ii, largePrefixSum.begin()-i+ii+small.size());

		if (outEnd[maxSampleEnd].real()/maxNormFactorEnd < outEnd[i].real()/normFactor) {
		    maxSampleEnd = i;
		    maxNormFactorEnd = normFactor;
		}
	    }
	    maxSamplesBegin[ii/small.size()+idxAdd] = small.size() - maxSampleBegin;
	    maxSamplesEnd[ii/small.size()+idxAdd] = small.size() - maxSampleEnd;
	}

	if (numberOfParts != pieces) break;

    }
    std::cout << "100%" << std::endl;
// #pragma omp parallel shared(small, large, maxSamplesBegin, maxSamplesEnd)
//     {
// #pragma omp for
//     }
//     // FIXME: special case.
//     // small size does not divide large size
//     // => last piece is not analysed.
//     // fix this.

    for (size_t i = 0; i < maxSamplesBegin.size()-1; ++i) {
	if (maxSamplesBegin[i] + maxSamplesEnd[i+1] <= small.size() &&
	    (maxSamplesBegin[i] + maxSamplesEnd[i+1] >= THRESHHOLD*small.size())) {
	    results.push_back((i+1)*small.size()-maxSamplesBegin[i]);
	}
    }
}

void printUsage() {
    std::cout << "Usage: ./soundMatch <needle.wav> <haystack.wav>" << std::endl;
}

/**
 * @param channel. Channel 0 is left, 1 is right.
 *
 */
inline void read(vector<int16_t> &res, size_t channel, FILE *fp, size_t sz) {
    uint8_t *buf = new uint8_t[sz*4];
    size_t read = fread(buf, 1, sz*4, fp);
    res.resize(sz);
    for (size_t i = 0; i < read; i+= 4) { //4 = 2bytes+2bytes, 2bytes pr channel and 2 channels.
	res[i/4] = getIntFromChars(buf[i+channel*2], buf[i+1+channel*2]);
    }
}


int main(int argc, char **argv) {

    std::vector<size_t> res;
    
    if (argc != 3) {printUsage(); return 1;}
 
    AudioFile needle(argv[1]);
    AudioFile haystack(argv[2]);

    //a1.getSamplesForChannel(0, a1in);
    //a2.getSamplesForChannel(0, a2in);

    match(needle, haystack, res);

    //a1in.clear();
    //a2in.clear();

    //match(a1Arr, a2Arr, a1Sz, a2Sz, res, spa1, spa2);
    if (res.size() == 0) {
	std::cout << "no matches found" << std::endl;
    } else {
	std::vector<std::string> resStr(res.size());
	for (size_t i = 0; i < res.size(); ++i) {
	    uint64_t second = res[i] / haystack.getSampleRate();

	    size_t secs = second % 60;
	    size_t mins = (second/60) % 60;
	    size_t hrs = second/3600;

	    std::stringstream ss;
	    if (hrs < 10)
		ss << "0";
	    ss << hrs << ":";
	    if (mins < 10)
		ss << "0";
	    ss << mins << ":";
	    if (secs < 10)
		ss << "0";
	    ss << secs;
	    resStr[i] = ss.str();
	}
	std::cout << "matches found starting at time [hh:mm:ss]: " << resStr << std::endl;
    }
}

#endif
