#include <vector>
#include <cmath>
#include <complex>

using std::complex;

class FTNaive {
public:
    static void FT(std::vector<complex<double> > &input, std::vector<complex<double> > &output, bool analysis = true);

    static void inverseFT(std::vector<complex<double> > &input, std::vector<complex<double> > &output);
};
