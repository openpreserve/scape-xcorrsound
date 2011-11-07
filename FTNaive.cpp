#include <vector>
#include <cmath>
#include <complex>

#include "FTNaive.h"

double const PI = 4*atan(1);

using namespace std;

void FTNaive::FT(vector<complex<double> > &input, vector<complex<double> > &output, bool analysis) {
    output.clear();
    output.reserve(input.size());
    const vector<complex<double> >::size_type N = input.size();

    for (vector<complex<double> >::size_type k = 0; k < N; ++k) {
	complex<double> Xk;
	for (vector<complex<double> >::size_type n = 0; n < N; ++n) {
	    complex<double> xn = input[n];
	    if (analysis) {
		complex<double> power(0.0,(-2*PI*k*n)/N);
		Xk += xn*exp(power);
	    } else {
		complex<double> power(0.0,(2*PI*k*n)/N);
		Xk += xn*exp(power);
	    }
	}

	if (!analysis) Xk /= N;

	output.push_back(Xk);
    }
}

void FTNaive::inverseFT(vector<complex<double> > &input, vector<complex<double> > &output) {
    FTNaive::FT(input,output,false);
}

