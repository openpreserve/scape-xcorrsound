#include <complex>
#include <fstream>
#include <iomanip>
#include <map>
#include <ostream>
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
using std::pair; using std::ofstream; using std::ifstream;

vector<uint32_t> db;
map<size_t, string> files;
int fileNumber = 0;

static const double PI = 3.14159265359;

static const size_t frameLength = 2048;//16384;

static const size_t advance = 512;

static const size_t sampleRate = 5512;

void writeDBToDisk(string filename) {

    ofstream of(filename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
    size_t init = of.tellp();
    for (size_t i = 0; i < db.size(); ++i) {
        of.write((char*) &db[i], sizeof(uint32_t));
    }

    of.close();

    string mapFilename = "map_" +filename;
    ofstream mof(mapFilename.c_str(), std::ios::out | std::ios::app);

    typedef map<size_t, string>::iterator m_iter;
    for (m_iter iter = files.begin(); iter != files.end(); ++iter) {
        mof << iter->first + init << " " << iter->second << endl;
    }

    mof.close();

}

void readDBFromDisk(string filename, vector<uint32_t> &out) {
    ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    out.clear();

    uint32_t buffer[(1<<20)];

    do {
        in.read((char*) buffer,(1<<20) * sizeof(uint32_t));
        for (size_t i = 0; i < in.gcount()/sizeof(uint32_t); ++i) {
            out.push_back(buffer[i]);
        }

    } while (!in.eof());

    in.close();

    string mapFilename = "map_" + filename;
    ifstream in_map(mapFilename.c_str(), std::ios::in);
    while (!in_map.eof()) {
        size_t idx; string filename;
        in_map >> idx >> filename;
        files.insert(make_pair(idx, filename));
    }
    in_map.close();

}

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

void generateFingerprintStream2(vector<int16_t> &input, vector<uint32_t> &output) {
    size_t begin = 0, end = frameLength;

    vector<double> hanningWindow;
    getHanningWindow(frameLength, hanningWindow);

    while (end < input.size()) {
        
        vector<double> frame(input.begin() + begin, input.begin() + end);
        
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

        uint32_t fp = (maxFreqIdx << 10) | secondIdx;
        //uint32_t fp = secondIdx;
        output.push_back(fp);
        begin += advance; end += advance;
    }

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

        std::swap(prevEnergy, energy);
        
        output.push_back(fingerprint);
    }
}

void query(const char *file) {

    AudioFile a(file);
    
    vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    vector<uint32_t> fingerprints;

    generateFingerprintStream2(samples, fingerprints);

    size_t bestMatch = db.size();
    size_t bestMatchDistance = fingerprints.size();
    cout << bestMatchDistance << endl;

    for (size_t i = 0; i < db.size() - fingerprints.size(); ++i) {
        uint32_t distance = 0;
        //for (size_t j = 0; j < fingerprints.size(); ++j) {
        for (size_t j = fingerprints.size()/2; j < fingerprints.size()/2+32; ++j) {
            uint32_t x = fingerprints[j] ^ db[i+j];
            int cnt = 0;
            while (x != 0) {
                cnt += (x&1);
                x >>= 1;
            }
            if (cnt > 0) ++distance;
        }
        if (distance < bestMatchDistance) {
            bestMatchDistance = distance;
            bestMatch = i;
        }
        if (distance < 20) {
            map<size_t, string>::iterator iter = files.lower_bound(i);

            string filename = iter->second;

            size_t start = 0;
            if (iter != files.begin()) {
                --iter;
                start = iter->first;
            }

            size_t sampleInFile = i - start;
    
            string timestamp = getTimestampFromSeconds(sampleInFile*advance / sampleRate);
            cout << "match in " << filename 
                 << " at " << timestamp
                 << " with distance " << distance << endl;
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

    generateFingerprintStream2(samples, fingerprints);

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
        cout << "db: " << db.size() << endl;
    }

    if (numDBfiles > 0) {
        writeDBToDisk("string.bin");
    } else {
        readDBFromDisk("string.bin", db);
    }

    cout << "db size: " << db.size() << endl;

    for (size_t i = numDBfiles+3; i < numDBfiles+numQueryfiles+3; ++i) {
        query(argv[i]);
    }

}
