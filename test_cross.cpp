#include "cross_correlation.h"
#include <vector>
#include <complex>
#include <iostream>

using namespace std;


int main() {
    int arr1[] = {1, 2, 3, 3, 7, 3};
    int arr2[] = {3, 2, 1, 4, 8, 4};
    vector<int> a(arr1, arr1+6);
    vector<int> b(arr2, arr2+6);
    vector<complex<double> > out;

    proxyFFT<int, double> ap(a);
    proxyFFT<int, double> bp(b);

    cross_correlation(ap, bp, out);

    for (int i = 0; i < out.size(); ++i) {
	cout << out[i].real() << " ";
    }
    cout << endl;
}
