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

void prefixSum(short *a, size_t sz, int64_t *res) {
    res[0] = a[0]*a[0];
    for (size_t i = 1; i < sz; ++i) {
	res[i] = res[i-1] + ((int64_t) a[i])*a[i];
    }
}

template<typename T>
void prefixSquareSum(vector<T> &a, vector<int64_t> &res) {
    res.resize(a.size());
    res[0] = a[0] * a[0];
    for (size_t i = 1; i < res.size(); ++i) {
	res[i] = res[i-1] + a[i] * a[i];
    }
}

double computeNormFactor(int64_t *spa1, int64_t *spa2, size_t a1Start, size_t a1End, size_t a2Start, size_t a2End) {
    return 0.5*(spa1[a1End]-((a1Start > 0)?spa1[a1Start-1]:0) + spa2[a2End]-((a2Start > 0)?spa2[a2Start-1]:0));
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
void match(T *s, T *l, size_t m, size_t n, std::vector<size_t> &res, int64_t *spa1, int64_t *spa2) {
    if (m > n) return;
    
    size_t prev = 0;
    //std::cout << "m: " << m << "    n: " << n << std::endl;
    for (size_t i = 0; i < n; i+=m) {
	std::vector<std::complex<double> > crossOut;

	if (i > 0) {
	    crossOut.clear();

	    size_t end = (i+m<n)?m:n-i;
	    
	    cross_correlation(s+m-end, l+i, end, end, crossOut);
	    
	    size_t maxSample = 0;
	    for (size_t j = 0; j < crossOut.size(); ++j) {
		double normj = computeNormFactor(spa1, spa2, j + m - end, m - 1, i, i + end - j - 1);
		double normm = computeNormFactor(spa1, spa2, m - end + maxSample, m - 1, i, i + end - maxSample - 1);
		// those indices are a pain to figure out.
		//std::cout << i+j << '\t' << crossOut[j].real() << '\t' << normj << std::endl;
		if (crossOut[j].real() / normj > crossOut[maxSample].real() / normm) {
		    maxSample = j;
		}
	    }
	    size_t lengthOfOverlap = end - maxSample;

	    //std::cout << "lengthB: " << lengthOfOverlap << std::endl;
	    if ((res.size() > 0 && res[res.size()-1] != i-m) || res.size() == 0) {
		if (lengthOfOverlap + prev <= m && (lengthOfOverlap+prev) >= THRESHHOLD*m) {
		    res.push_back(i-prev);
		} 
	    }
	    std::cout << lengthOfOverlap << std::endl;
	    if (lengthOfOverlap >= THRESHHOLD*m) {
		res.push_back(i+maxSample+m-end);
	    }
	}

	crossOut.clear();
	//std::cout << "computing cross: l+" << i << ", s, " << ((i+m<n)?m:n-i) << ", " << m << std::endl;
	size_t end = (i+m<n)?m:n-i;
	cross_correlation(l+i, s, end, end, crossOut);

	size_t maxSample = 0;
	for (size_t j = 0; j < crossOut.size(); ++j) {
	    double normj = computeNormFactor(spa1, spa2, 0, end-j-1, i+j, std::min(i+m,n-1));
	    double normm = computeNormFactor(spa1, spa2, 0, end-maxSample-1, i+maxSample, std::min(i+m, n-1));
	    //std::cout << i+j << '\t' << crossOut[j].real() << '\t' << normj << std::endl;	    
	    if (crossOut[j].real() / normj > crossOut[maxSample].real() / normm) {
		maxSample = j;
	    }
	}
	

	
	maxSample = m - maxSample;
	std::cout << i/m << ": \t" << maxSample << " ";

	prev = maxSample; // length

	if (maxSample >= ((double) m)*0.9) {
	    std::cout << "maxsample==0.9m, match..: " << i << std::endl;
	    res.push_back(i);
	}
    }
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

    // AudioFile a1("/home/jsn/jingle-radioavis.wav");
    // AudioFile a2("/home/jsn/test2.wav");
    
    AudioFile a1(argv[1]);
    AudioFile a2(argv[2]);

    std::vector<short> a1in;
    std::vector<short> a2in;
    a1.getSamplesForChannel(0, a1in);
    a2.getSamplesForChannel(0, a2in);
    size_t a1Sz = a1in.size(); size_t a2Sz = a2in.size();
    short *a1Arr = new short[a1Sz];
    short *a2Arr = new short[a2Sz];
    int64_t *spa1 = new int64_t[a1Sz];
    int64_t *spa2 = new int64_t[a2Sz];

    for (size_t i = 0; i < a1in.size(); ++i) {
    	a1Arr[i] = a1in[i];
    }
    for (size_t i = 0; i < a2in.size(); ++i) {
    	a2Arr[i] = a2in[i];
    }

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
