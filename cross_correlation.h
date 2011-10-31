#ifndef CROSS_CORRELATION_GUARD
#define CROSS_CORRELATION_GUARD

#include <algorithm>
#include <fftw3.h>
#include <vector>
#include <iterator>
#include <complex>

template<typename T1, typename T2>
struct proxyFFT {

    std::vector<T1> _array;
    std::vector<std::complex<T2> > _transform;
    bool _computed;
    
    template<typename T>
    proxyFFT(std::vector<T> &array) : _array(array), _computed(false) {};

    proxyFFT(typename std::vector<T1>::iterator begin, typename std::vector<T1>::iterator end) : _computed(false) {
	_array.clear();
	_array.insert(_array.begin(), begin, end);
    };

    void transform() {
	if (_computed) return;

	fftw_plan plan;
	fftw_complex *t = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _array.size()*2);
	fftw_complex *o = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * _array.size()*2);
	for (size_t i = 0; i < _array.size(); ++i) {
	    t[i][0] = _array[i];
	    t[i][1] = 0;
	}
	for (size_t i = _array.size(); i < _array.size()*2; ++i) {
	    t[i][0] = 0;
	    t[i][1] = 0;
	}
	plan = fftw_plan_dft_1d(_array.size()*2, t, o, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(plan);
	fftw_free(t);
	_transform.resize(_array.size());
	for (size_t i = 0; i < _array.size(); ++i) {
	    _transform[i] = std::complex<T2>(o[i][0],o[i][1]);		
	}
	fftw_free(o);
	_computed = true;
    }

    std::vector<std::complex<T2> >& getTransform() {
	transform();
	return _transform;
    } 

};

/**
 * @param b is 'shifted' along @param a. That is, sample b[0] is always 
 * part of the product and a[n] is also always part of the product.
 *
 * first round:
 * a [ 0 1 2 ... n ]
 * b [ 0 1 2 ... n ]
 *
 * second round:
 * a [ 0 1 2 ... n ]
 * b   [ 0 1 2 ... n ]
 *
 * third round:
 * a [ 0 1 2 ... n ]
 * b     [ 0 1 2 ... n ]
 *
 * etc..
 */

template<typename T1, typename T2>
void cross_correlation(proxyFFT<T1, T2> &a, proxyFFT<T1, T2> &b, std::vector<std::complex<T2> > &out) {
    std::vector<std::complex<T2> > out2 = a.getTransform();
    std::vector<std::complex<T2> > out1 = b.getTransform();
    size_t prodSize = std::min(out2.size(), out1.size())*2;
    fftw_complex *prod = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * prodSize);
    fftw_complex *invprod = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * prodSize);

    for (size_t i = 0 ; i < prodSize/2; ++i) {
	prod[i][0] = out1[i].real() * out2[i].real() - out1[i].imag() * out2[i].imag();
	prod[i][1] = out1[i].real() * out2[i].imag() + out1[i].imag() * out2[i].real();
    }
    for (size_t i = prodSize/2; i < prodSize; ++i) {
	prod[i][0] = 0;
	prod[i][1] = 0;
    }

    fftw_plan plan= fftw_plan_dft_1d(prodSize, prod, invprod, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    
    out.resize(prodSize/2);
    for (size_t i = 0; i < out.size(); ++i) {
	out[i] = std::complex<T2>(invprod[i][0]/2/out.size(), invprod[i][1]);
    }
    fftw_free(prod); fftw_free(invprod);
}

template<typename in_type, typename out_type>
void cross_correlation(in_type *a1, in_type *a2, size_t a1_size, size_t a2_size, std::vector<out_type> &out) {
    fftw_complex *in1,*in2, *in3, *out1, *out2, *out3;
    fftw_plan p1,p2,p3;
  
    in1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a2_size*2);
    in2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a1_size*2);
    in3 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * std::min(a1_size,a2_size)*2);
    out1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a2_size*2);
    out2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a1_size*2);
    out3 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * std::min(a1_size,a2_size)*2);

    for (size_t i = 0; i < a1_size; ++i) {
	in2[i][0] = a1[i];
	in2[i][1] = 0;
    }
    
    for (size_t i = a1_size; i < a1_size*2; ++i) {
	in2[i][0] = 0;
	in2[i][1] = 0;
    }

    for (size_t i = 0; i < a2_size; ++i) {
	in1[i][0] = a2[i];
	in1[i][1] = 0;
    }

    for (size_t i = a2_size; i < a2_size*2; ++i) {
	in1[i][0] = 0;
	in1[i][1] = 0;
    }

    p1 = fftw_plan_dft_1d(a2_size*2, in1, out1, FFTW_FORWARD, FFTW_ESTIMATE);
    p2 = fftw_plan_dft_1d(a1_size*2, in2, out2, FFTW_FORWARD, FFTW_ESTIMATE);
    p3 = fftw_plan_dft_1d(std::min(a1_size,a2_size)*2, in3, out3, FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(p1);
    fftw_free(in1);
    fftw_execute(p2);
    fftw_free(in2);
    
    for (size_t i = 0; i < std::min(a1_size,a2_size)*2; ++i) {
	in3[i][0] = out1[i][0] * out2[i][0] + out1[i][1] * out2[i][1];
	in3[i][1] = -out1[i][0] * out2[i][1] + out1[i][1] * out2[i][0];
    }
    fftw_free(out1);
    fftw_free(out2);

    fftw_execute(p3);

    fftw_destroy_plan(p1);
    fftw_destroy_plan(p2);
    fftw_destroy_plan(p3);

    out.resize(std::min(a1_size,a2_size));
    for (size_t i = 0; i < out.size(); ++i) {
	//out[i] = complex_type(out3[i][0]/out.size(),out3[i][1]);
	out[i] = out_type(out3[i][0]/out.size()/2,out3[i][1]);
    }
    fftw_free(out3);
}

#endif
