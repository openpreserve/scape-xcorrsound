#include "cross_correlation.h"
#include <iostream>
#include <complex>

using namespace std;

void naive(int *a1, int *a2, vector<int> &out) {
    out.clear();

    for (int i = 0; i < 8; ++i) {
	int sum = 0;
	for (int j = 0; j+i<8; ++j) {
	    sum += a2[j]*a1[j+i];
	}
	out[i] = sum;
    }
}

int main() {
    int a1[8] = {9,6,1,0,5,8,0,8};
    int a2[8] = {8,6,2,4,8,1,9,2};

    vector<complex<double> > output(8,complex<double>(0.0,0.0));
    cross_correlation(a1, a2, 8, 8, output);
    for (vector<complex<double> >::iterator it = output.begin(); it != output.end(); ++it) {
	cout << *it << endl;
    }

    vector<int> output2(8,0);
    naive(a1, a2, output2);
    for (int i = 0; i < 8; ++i) {
	cout << output2[i] << endl;
    }
}
