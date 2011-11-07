#include <vector>
#include <cmath>
#include <complex>

using std::complex;

template<typename T>
class FFT {

public:
    typedef typename std::vector<complex<T> >::iterator iter;
    
    void FT(std::vector<complex<T> > &input, std::vector<complex<T> > &output, bool analysis = true);

    void inverseFT(std::vector<complex<T> > &input, std::vector<complex<T> > &output);

private:
    static int getNextPowerOfTwo(size_t n);
    static bool isPowerOfTwo(size_t n);

//    static void FTrec(const std::vector<complex<double> > &input, std::vector<complex<double> > &output, bool analysis, int idxJump, int startIdx, iter &X, iter &E, int N);
    static void FTrec(iter input, int N, int jump, iter X, iter E, bool analysis);

};

