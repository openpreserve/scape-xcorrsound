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

static const double THRESHHOLD = 0.98;

using std::vector; using std::complex; using std::cout; using std::endl;

template<typename T>
void prefixSquareSum(vector<T> &a, vector<int64_t> &res) {
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

template<typename T>
void match(vector<T> &small, vector<T> &large, vector<size_t> &results) {
    vector<int64_t> smallPrefixSum, largePrefixSum;
    prefixSquareSum(small, smallPrefixSum);
    prefixSquareSum(large, largePrefixSum);

    size_t numberOfParts = large.size()/small.size();
    
    proxyFFT<T, double> smallFFT(small);

    vector<int64_t> maxSamplesBegin(numberOfParts);
    vector<int64_t> maxSamplesEnd(numberOfParts);

    for (size_t ii = 0; ii < numberOfParts*small.size(); ii+=small.size()) {
	proxyFFT<T, double> largeFFT(large.begin()+ii, large.begin()+ii+small.size());
	vector<complex<double> > outBegin;
	vector<complex<double> > outEnd;
	
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

	maxSamplesBegin[ii/small.size()] = small.size() - maxSampleBegin;
	maxSamplesEnd[ii/small.size()] = small.size() - maxSampleEnd;
    }

    // FIXME: special case.
    // small size does not divide large size
    // fix this.

    for (size_t i = 0; i < maxSamplesBegin.size()-1; ++i) {
	if (maxSamplesBegin[i] + maxSamplesEnd[i+1] <= small.size() &&
	    (maxSamplesBegin[i] + maxSamplesEnd[i+1] >= 0.90*small.size())) {
	    results.push_back((i+1)*small.size()-maxSamplesBegin[i]);
	}
    }
}

void printUsage() {
    std::cout << "Usage: ./soundMatch <needle.wav> <haystack.wav>" << std::endl;
}

int main(int argc, char **argv) {

    std::vector<size_t> res;
    
    if (argc != 3) {printUsage(); return 1;}
 
    AudioFile a1(argv[1]);
    AudioFile a2(argv[2]);

    std::vector<short> a1in;
    std::vector<short> a2in;
    a1.getSamplesForChannel(0, a1in);
    a2.getSamplesForChannel(0, a2in);

    prefixSum(a1Arr, a1Sz, spa1);
    prefixSum(a2Arr, a2Sz, spa2);

    match(a1in, a2in, res);

    a1in.clear();
    a2in.clear();

    //match(a1Arr, a2Arr, a1Sz, a2Sz, res, spa1, spa2);
    if (res.size() == 0) {
	std::cout << "no matches found" << std::endl;
    } else {
	std::vector<std::string> resStr(res.size());
	for (size_t i = 0; i < res.size(); ++i) {
	    uint64_t second = res[i] / a1.getSampleRate();

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
    // delete[] a1Arr;
    // delete[] a2Arr;
    // delete[] spa1;
    // delete[] spa2;
}

#endif
