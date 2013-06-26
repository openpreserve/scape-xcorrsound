#include <vector>
#include <cmath>
#include <complex>
#include <iostream>
#include "FFT.h"

double const PI = 4*atan(1);
int cnt = 0;
using namespace std;

template<typename T>
bool FFT<T>::isPowerOfTwo(size_t n) {
    bool found = false;
    for (int i = 0 ; i < 32; ++i) {
	if (((n & (1<<i)) >> i) == 1) {
	    if (found) return false;
	    found = true;
	}
    }

    return true;

}

template<typename T>
int FFT<T>::getNextPowerOfTwo(size_t n) {
    for (int i = 31; i >= 0; --i) {
	if ((n>>i) == 1) {
	    return (1<<(i+1));
	}
    }
    return 0;
}

template<typename T>
void FFT<T>::FT(std::vector<complex<T> > &input, std::vector<complex<T> > &output, bool analysis) {
    if (!isPowerOfTwo(input.size())) {
	int next = getNextPowerOfTwo(input.size());
	for (typename vector<complex<T> >::size_type i = input.size(); i < next; ++i) {
	    input.push_back(complex<T>(0.0,0.0));
	}
    }
    output.clear();
    output.resize(input.size());

    vector<complex<T> > additionalSpace(input.size());
    iter ob = output.begin();
    iter as = additionalSpace.begin();
    //FTrec(input,output,analysis,1,0,ob,as,output.size());
    FTrec(input.begin(), output.size(), 1, output.begin(), additionalSpace.begin(), analysis);
    if (!analysis) {
	for (typename vector<complex<T> >::size_type i = 0; i < output.size(); ++i) {
	    output[i] /= output.size();
	}
    }
}

template<typename T>
void FFT<T>::FTrec(iter input, int N, int jump, iter X, iter even, bool analysis) {

    iter odd = (even + N/2);
    
    if (N == 1) {
	//output[0] = input[startIdx];
	*X = *input;
	return;
    }

    FTrec(input, N/2, jump*2, even, X, analysis);
    FTrec(input+jump, N/2, jump*2, odd, X, analysis);

    for (typename vector<complex<T> >::size_type k = 0; k < N/2; ++k) {
	if (analysis) {
	    *(odd+k) *= exp(complex<T>(0.0,(-2.0*PI*k)/N));
	} else {
	    *(odd+k) *= exp(complex<T>(0.0,(2.0*PI*k)/N));
	}
    }

    for (typename vector<complex<T> >::size_type k = 0; k < N/2; ++k) {
	*(X+k) = *(even+k) + *(odd+k);
	*(X+k+N/2) = *(even+k) - *(odd+k);
    }
}

template<typename T>
void FFT<T>::inverseFT(vector<complex<T> > &input, vector<complex<T> > &output) {
    FT(input,output,false);
}

template class FFT<double>;
template class FFT<long double>;
//template class FFT<__float128>;
