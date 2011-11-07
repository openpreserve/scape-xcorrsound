#include "FFT.h"
#include <iostream>

#define eps 1e-3

using namespace std;

void printError(vector<complex<double> > &expected, vector<complex<double> > &received) {
    if (expected.size() != received.size()) {
	cout << "Expected and received size differ" << endl; 
	return;
    }

    for (size_t i = 0 ; i < expected.size(); ++i) {
	cout << "expected: " << expected[i].real() << " + " << expected[i].imag() << "i      ";
	cout << "received: " << received[i].real() << " + " << received[i].imag() << "i" << endl;
    }
}

bool checkEqual(vector<complex<double> > &a, vector<complex<double> > &b) {
    if (a.size() != b.size()) return false;

    for (size_t i = 0; i < a.size(); ++i) {
	if (!(fabs(a[i].real()-b[i].real()) < eps && fabs(a[i].imag()-b[i].imag()) < eps)) {
	    return false;
	}
    }
    return true;
}

bool test1() {
    FFT fft;
    vector<complex<double> > in, out, expected, temp;
    in.push_back(complex<double>(1.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));

    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));
    expected.push_back(complex<double>(0.125,0.0));

    fft.FT(in,out);
    
    if (!checkEqual(expected,out)) {
	printError(expected,out);
	return false;
    } 

    fft.inverseFT(out, temp);

    if (!checkEqual(in,temp)) {
	printError(in,temp);
	return false;
    }

    return true;
    
}

bool test2() {
    FFT fft;

    vector<complex<double> > in, out, expected, temp;
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(1.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));

    expected.push_back(complex<double>( 0.125,  0.0));
    expected.push_back(complex<double>( 0.088, -0.088));
    expected.push_back(complex<double>( 0.000, -0.125));
    expected.push_back(complex<double>(-0.088, -0.088));
    expected.push_back(complex<double>(-0.125,  0.0));
    expected.push_back(complex<double>(-0.088,  0.088));
    expected.push_back(complex<double>( 0.000,  0.125));
    expected.push_back(complex<double>( 0.088,  0.088));

    fft.FT(in,out);
    
    if (!checkEqual(expected,out)) {
	printError(expected,out);
	return false;
    } 
    
    fft.inverseFT(out, temp);

    if (!checkEqual(in,temp)) {
	printError(in,temp);
	return false;
    }
    return true;
}

bool test3() {
    FFT fft;

    vector<complex<double> > in, out, expected, temp;
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(1.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));
    in.push_back(complex<double>(0.0,0.0));

    expected.push_back(complex<double>( 0.125,  0.0));
    expected.push_back(complex<double>( 0.088, -0.088));
    expected.push_back(complex<double>( 0.000, -0.125));
    expected.push_back(complex<double>(-0.088, -0.088));
    expected.push_back(complex<double>(-0.125,  0.0));
    expected.push_back(complex<double>(-0.088,  0.088));
    expected.push_back(complex<double>( 0.000,  0.125));
    expected.push_back(complex<double>( 0.088,  0.088));

    fft.FT(in,out);
    
    if (!checkEqual(expected,out)) {
	printError(expected,out);
	cout << __FILE__ << ": " << __LINE__ << endl;
	return false;
    } 
    
    fft.inverseFT(out, temp);

    return true;

}

int main() {
    if (!test1()) {
	cout << "test1 failed" << endl;
    } else {
	cout << "test1 succeeded" << endl;
    }

    if (!test2()) {
	cout << "test2 failed" << endl;
    } else {
	cout << "test2 succeeded" << endl;
    }

    if (!test3()) {
	cout << "test3 failed" << endl;
    } else {
	cout << "test3 succeeded" << endl;
    }
}
