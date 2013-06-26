#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "../AudioFile.h"
#include "computeFFT.h"
#include "../my_utils.h"

static const size_t advance = 512;

static const size_t frameLength = 2048;

using std::string; using std::map; using std::vector;
using std::complex; using std::cout; using std::endl;
using std::setw;

void find_freqs(char *filename, map<uint32_t, uint32_t> &res) {

    AudioFile a(filename);
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    size_t begin = 0, end = frameLength;

    vector<double> hanningWindow;
    getHanningWindow(frameLength, hanningWindow);

    while (end < samples.size()) {
        
        vector<double> frame(samples.begin() + begin, samples.begin() + end);
        
        double rms = 0.0;
        for (size_t i = 0; i < frame.size(); ++i) {
            rms += frame[i]*frame[i];
        }
        rms /= frameLength;
        rms = sqrt(rms);
        
        for (size_t i = 0; i < frame.size(); ++i) {
            frame[i] = (frame[i]/rms) * hanningWindow[i];
        }

        vector<complex<double> > transform;
        computeFFT(frame, transform);

        double maxFreq = 0.0;
        size_t maxFreqIdx = 0;
        size_t secondIdx = 0;
        // hz = idx * sampleRate/frameLength
        // idx = hz * frameLength / sampleRate
        size_t val = ceil(318.0*frameLength/5512.0);
        for (size_t i = 0; i < transform.size()/2; ++i) {
            if (abs(transform[i]) > maxFreq) {
                secondIdx = maxFreqIdx;
                maxFreqIdx = i;
                maxFreq = abs(transform[i]);
            }
            //if (i < 10) cout << setw(8) << abs(transform[i]);
        }
        //cout << endl;

        //uint32_t fp = (maxFreqIdx << 10) | secondIdx;
        uint32_t fp = secondIdx;
        if (res.find(fp) != res.end()) {
            res[fp] = res[maxFreqIdx] + 1;
        } else {
            res[fp] = 1;
        }

        begin += advance; end += advance;
    }

}


int main(int argc, char *argv[]) {

    if (argc < 2) {
        return 1;
    }

    map<uint32_t, uint32_t> res;

    for (size_t i = 1; i < argc; ++i) {
        find_freqs(argv[i], res);
    }
    
    typedef map<uint32_t, uint32_t>::iterator iter;
    
    for (iter i = res.begin(); i != res.end(); ++i) {
        std::cout << i->first << '\t' << i->second << std::endl;
    }

}
