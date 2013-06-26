#ifndef SOUND_MATCH_H
#define SOUND_MATCH_H

#include <vector>
#include <complex>
#include <iostream>
#include <algorithm>
#include "cross_correlation.h"

template<typename T>
void match(T *s, T *l, size_t m, size_t n, std::vector<size_t> &res) {

    if (m > n) return;
    
    T *sInv = new T[m];
    for (size_t i = 0; i < m; ++i) {
	sInv[i] = s[m-i-1];
    }
    
    size_t prev = 0;

    for (size_t i = 0; i < n; i+=m) {
	std::vector<std::complex<double> > crossOut;

	if (i > 0) {
	    cross_correlation(l+i*m, sInv, ((i+1)*m>n)?n:m, m, crossOut);
	    size_t maxSample = 0;
	    for (size_t j = 0; i < crossOut.size(); ++j) {
		if (crossOut[j].real() > crossOut[maxSample].real()) {
		    maxSample = j;
		}
	    }
	    
	    if (prev + maxSample > m-1000) {
		std::cout << "match...: " << i-prev << std::endl;
		res.push_back(i-prev);
	    }
	    
	}

	cross_correlation(s, l+i*m, m, std::min((i+1)*m,n), crossOut);
	size_t maxSample = 0;
	for (size_t j = 0; i < crossOut.size(); ++j) {
	    if (crossOut[j].real() > crossOut[maxSample].real()) {
		maxSample = j;
	    }
	}
	
	if (maxSample == 0) {
	    std::cout << "match..: " << i << std::endl;
	    res.push_back(i);
	}

	prev = maxSample;
    }

    
}

int main() {

    int small[3] = {10,20,30};
    int large[15] = {0, -3, -50, 30, -20, 10, 20, 30, 40, -20, -10, 33, 22, 30, -15};
    std::vector<size_t> res;
    match(small, large, 3, 15, res);
    for (size_t i = 0; i < res.size(); ++i) {
	std::cout << i << std::endl;
    }
}

#endif
