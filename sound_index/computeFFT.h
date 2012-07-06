#ifndef COMPUTEFFT_H
#define COMPUTEFFT_H

#include <vector>
#include <complex>

#include <fftw3.h>


template<typename T1, typename T2>
inline
void computeFFT(std::vector<T1> &input, std::vector<std::complex<T2> > &output) {
    using std::vector; using std::complex;

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

#endif
