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

void prefixSum(short *a, size_t sz, int64_t *res) {
    res[0] = a[0]*a[0];
    for (size_t i = 1; i < sz; ++i) {
	res[i] = res[i-1] + ((int64_t) a[i])*a[i];
    }
}

double computeNormFactor(int64_t *spa1, int64_t *spa2, size_t a1Start, size_t a1End, size_t a2Start, size_t a2End) {
    return 0.5*(spa1[a1End]-((a1Start > 0)?spa1[a1Start-1]:0) + spa2[a2End]-((a2Start > 0)?spa2[a2Start-1]:0));
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

	//std::cout << "lengthA: " << maxSample << std::endl;

	//std::cout << crossOut << std::endl;
	prev = maxSample; // length
	//std::cout << "prev: " << prev << std::endl;
	//std::cout << " ---- " << std::endl;

	if (maxSample >= ((double) m)*0.9) {
	    std::cout << "maxsample==0.9m, match..: " << i << std::endl;
	    res.push_back(i);
	}


    }

    
}

template<typename T>
void match(vector<T> &small, vector<T> &large, 
	   vector<uint64_t> &prefixSumSmall, vector<uint64_t> &prefixSumLarge, 
	   vector<size_t> results, ) {
    
    size_t numberOfParts = large.size()/small.size();
    
    proxyFFT<double, double> smallFFT(small);

    for (size_t i = 0; i < numberOfParts*small.size(); i+=small.size()) {
	
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

    a1in.clear();
    a2in.clear();

    match(a1Arr, a2Arr, a1Sz, a2Sz, res, spa1, spa2);
    if (res.size() == 0) {
	std::cout << "no matches found" << std::endl;
    } else {
	std::cout << "matches found starting at sample: " << res << std::endl;
    }
    delete[] a1Arr;
    delete[] a2Arr;
    delete[] spa1;
    delete[] spa2;
}

#endif
