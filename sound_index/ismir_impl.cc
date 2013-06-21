#include <complex>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "stdint.h"

#include <fftw3.h>

#include "../AudioStream.h"
#include "../AudioFile.h"
#include "computeFFT.h"
#include "../my_utils.h"

#include <cstring>

using std::vector; using std::complex; using std::map; 
using std::set; using std::string; using std::stringstream;
using std::cin; using std::cout; using std::endl;
using std::pair;

vector<uint32_t> db;
map<size_t, string> files;
int fileNumber = 0;

static const double PI = 3.14159265359;

static const size_t frameLength = 16384;

static const size_t advance = 512;

static const size_t sampleRate = 5512;

bool flip = false;

inline
static double normalize(vector<double> &samples) {

    double rootMeanSquare = 0.0;
    for (size_t i = 0; i < samples.size(); ++i) {
        rootMeanSquare += samples[i] * samples[i];
    }

    rootMeanSquare /= samples.size();

    return sqrt(rootMeanSquare);

}

inline
static double getHz(const size_t idx) {
    if (idx > frameLength) return frameLength;

    double T = static_cast<double>(frameLength) / static_cast<double>(sampleRate);
    return idx / T;
}

inline
static double getMel(const double hz) {
    return 2595 * std::log10(1+hz/700.0);
}

inline
static void getHanningWindow(size_t windowLength, vector<double> &window) {

    window.resize(windowLength);
    
    for (size_t i = 0; i < windowLength; ++i) {
        window[i] = (25.0/46.0) - (21.0/46.0) * cos((2*PI*i)/(windowLength-1));
    }

}

inline
static size_t getIndexFromHz(double hz) {

    // hz = idx*(sampleRate/frameLength)
    // => idx = ceil(hz * (frameLength / sampleRate))
    size_t idx = (size_t) hz * static_cast<double>(frameLength) / static_cast<double>(sampleRate);
    return idx;
}

inline
static void getBarkScale(double maxFrequency, vector<size_t> &indices) {
    
    size_t bands = 33;
    indices.resize(bands);

    double logMin = log(318.0) / log(2);
    double logMax = log(maxFrequency) / log(2);

    double delta = (logMax - logMin) / bands;
    
    double sum = 0.0;
    for (size_t i = 0; i < bands; ++i) {
        
        double hz = pow(2, logMin + sum);
        
        indices[i] = getIndexFromHz(hz);

        sum += delta;
    }
    
}

inline static void getLogScale(double maxFrequency, vector<size_t> &indices) {
    size_t bands = 33;
    indices.resize(bands);

    double logMin = log(318.0) / log(2);
    double logMax = log(maxFrequency) / log(2);

    double delta = (logMax - logMin) / bands;
    
    double sum = 0.0;
    for (size_t i = 0; i < bands; ++i) {
        
        double hz = pow(2, logMin + sum);
        
        indices[i] = getIndexFromHz(hz);

        sum += delta;
    }
}

inline
static uint32_t getFingerprint(vector<double> &prevEnergy, vector<double> &energy) {
    
    uint32_t fingerprint = 0;

    for (size_t bitPos = 0; bitPos < 32; ++bitPos) {
        double val = energy[bitPos] - energy[bitPos+1] - (prevEnergy[bitPos] - prevEnergy[bitPos+1]);
        
        uint32_t bit = (val > 0)?1:0;

        fingerprint = fingerprint + (bit << bitPos);

    }

    return fingerprint;
}

void generateFingerprintStream(vector<int16_t> &input, vector<uint32_t> &output) {
    vector<double> hanningWindow; 
    vector<size_t> logScale;
    
    getHanningWindow(frameLength, hanningWindow);

    getLogScale(2000, logScale);

    vector<double> prevEnergy(33, 0.0);
    size_t frameStart, frameEnd;
    frameStart = 0; frameEnd = frameStart += advance;

    for (size_t frameStart = 0; frameEnd < input.size(); frameStart += advance, frameEnd += advance) {
        vector<complex<double> > transform(frameLength);
        
        vector<double> tmp(frameLength);
        for (size_t i = 0; i < frameLength; ++i) {
            tmp[i] = input[frameStart+i];
        }
        normalize(tmp);
        for (size_t i = 0; i < frameLength; ++i) {
            tmp[i] = tmp[i] * hanningWindow[i];
        }

        computeFFT(tmp, transform);
        
        vector<double> energy(33, 0.0);
        
        for (size_t i = 0; i < logScale.size()-1; ++i) {

            double absVal = 0.0;
            for (size_t j = logScale[i]; j < logScale[i+1]; ++j) {
                absVal += abs(transform[j]);
            }

            energy[i] = absVal/(logScale[i+1]-logScale[i]);
        }
   
        uint32_t fingerprint = getFingerprint(prevEnergy, energy);
        //if (flip) cout << fingerprint << endl;
        std::swap(prevEnergy, energy);
        
        output.push_back(fingerprint);
    }
}

void query(const char *file) {

    AudioFile a(file);
    
    vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);

    size_t bestMatch = db.size();
    size_t bestMatchDistance = fingerprints.size();
    cout << bestMatchDistance << endl;

    for (size_t i = 0; i < db.size() - fingerprints.size(); ++i) {
        uint32_t distance = 0;
        for (size_t j = 0; j < fingerprints.size(); ++j) {
            uint32_t x = fingerprints[j] ^ db[i+j];
            int cnt = 0;
            while (x != 0) {
                cnt += (x&1);
                x >>= 1;
            }
            if (cnt > 2) ++distance;
        }
        if (distance < bestMatchDistance) {
            bestMatchDistance = distance;
            bestMatch = i;
        }
    }
    
    map<size_t, string>::iterator iter = files.lower_bound(bestMatch);

    string filename = iter->second;

    size_t start = 0;
    if (iter != files.begin()) {
        --iter;
        start = iter->first;
    }
    
    size_t sampleInFile = bestMatch - start;
    
    string timestamp = getTimestampFromSeconds(sampleInFile*advance / sampleRate);

    cout << "best match in " << filename << " at " << timestamp << " with distance " << bestMatchDistance << endl;
    cout << sampleInFile << endl;
}

void insert_file(const char *file) {

    AudioFile a(file);
    
    vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);

    db.insert(db.end(), fingerprints.begin(), fingerprints.end());

    files.insert(make_pair(db.size(), string(file)));
    
}


int main(int argc, char *argv[]) {

    if (argc < 2) {
        return 1;
    }

    if (strcmp(argv[1], "test") == 0) {
        vector<size_t> barks(33);
        getLogScale(sampleRate/2.0, barks);
        for (size_t i = 0; i < barks.size(); ++i) {
            cout << barks[i] << endl;
        }
        return 0;
    }
    
    size_t numDBfiles = static_cast<size_t>(atoi(argv[1]));
    size_t numQueryfiles = static_cast<size_t>(atoi(argv[2]));

    for (size_t i = 3; i < numDBfiles+3; ++i) {
        insert_file(argv[i]);
    }
    flip = true;
    cout << "db size: " << db.size() << endl;

    for (size_t i = numDBfiles+3; i < numDBfiles+numQueryfiles+3; ++i) {
        query(argv[i]);
    }

}
