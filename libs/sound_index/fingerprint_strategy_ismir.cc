#include <AudioFile.h>
#include <cmath>
#include <complex>
#include <computeFFT.h>
#include <cstdlib>
#include <ctime>
#include <fingerprint_strategy.hh>
#include <functional>
#include <sstream>
#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <wavStdinReader.hh>
#include <iomanip>

namespace {
    size_t frameLength = 2048;

    size_t advance = 64;

    size_t sampleRate  = 5512;

    std::string getTmpDir() {
        const char *val = ::getenv("TmpSoundIndex");
        if (val == 0) {
            return "/tmp/";
        } else {
            return std::string(val);
        }
    }

    std::string getTmpFilename() {

        timeval tv;
        gettimeofday(&tv, 0);

        std::stringstream ss;
        for (size_t i = 0; i < sizeof(tv.tv_sec); ++i) {
            char c = ((tv.tv_sec >> (i*8) & 255) % 26)+65;
            ss << c;
        }

        for (size_t i = 0; i < sizeof(tv.tv_usec); ++i) {
            char c = ((tv.tv_usec >> (i*8) & 255) % 26)+97;
            ss << c;
        }
        ss << ".wav";

        return ss.str();
    }

    void getHanningWindow(size_t windowLength, std::vector<double> &window) {

        const double PI = 3.14159265359;

        window.resize(windowLength);
    
        for (size_t i = 0; i < windowLength; ++i) {
            window[i] = (25.0/46.0) - (21.0/46.0) * cos((2*PI*i)/(windowLength-1));
        }
    }

    template <typename T>
    inline
    static double normalize(std::vector<T> &samples) {

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
    static void getBarkScale(double maxFrequency, std::vector<size_t> &indices) {
    
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
    static void getLogScale(double maxFrequency, std::vector<size_t> &indices) {
        size_t bands = 33;
        indices.resize(bands);

        double logMin = log(318.0) / log(2);
        double logMax = log(maxFrequency) / log(2);

        double delta = (logMax - logMin) / bands; //linear increase on log scale
    
        double sum = 0.0;
        for (size_t i = 0; i < bands; ++i) {
        
            double hz = pow(2, logMin + sum);
        
            indices[i] = getIndexFromHz(hz);

            sum += delta;
        }
    }

    inline
    static uint32_t getFingerprint(std::vector<double> &prevEnergy, std::vector<double> &energy) {
    
        uint32_t fingerprint = 0;

        for (size_t bitPos = 0; bitPos < 32; ++bitPos) {
            double val = energy[bitPos] - energy[bitPos+1] - (prevEnergy[bitPos] - prevEnergy[bitPos+1]);
        
            uint32_t bit = (val > 0)?1:0;

            fingerprint = fingerprint + (bit << bitPos);

        }

        return fingerprint;
    }

    void generateFingerprintStream(std::vector<int16_t> &input, std::vector<uint32_t> &output) {
        std::vector<double> hanningWindow; 
        std::vector<size_t> logScale;
    
        getHanningWindow(frameLength, hanningWindow);

        getLogScale(2000, logScale);

        std::vector<double> prevEnergy(33, 0.0);
        size_t frameStart, frameEnd;

        frameStart = 0; frameEnd = frameStart + frameLength;
        for (size_t frameStart = 0; frameEnd < input.size(); frameStart += advance, frameEnd += advance) {
            std::vector<std::complex<double> > transform(frameLength);
        
            std::vector<double> tmp(frameLength);
            for (size_t i = 0; i < frameLength; ++i) {
                tmp[i] = input[frameStart+i];
            }
            normalize(tmp);
            for (size_t i = 0; i < frameLength; ++i) {
                tmp[i] = tmp[i] * hanningWindow[i];
            }

            computeFFT(tmp, transform);
        
            std::vector<double> energy(33, 0.0);
        
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
}

namespace sound_index {

    void 
    fingerprint_strategy_ismir::getFingerprintsForFile(std::string filename, std::vector<uint32_t> &res) {
        std::vector<int16_t> samples;

        // if filename is not wav file, start by converting to 5512hz stereo wav file
        // if filename is on stdin, assume it conforms to assumption above.
        // otherwise assume the file is 5512hz wav file.
        if (filename == "-") {
            // use stdin wav file reader.
            
            wavStdinReader wsr;
            wsr.getSamplesForChannel(0, samples);

        } else if (filename.substr(filename.size()-3, 3) != "wav") {
            std::stringstream tmpss;

            AudioFile a(tmpss.str().c_str());
            AudioStream as(a.getStream(0));

            as.read(a.getNumberOfSamplesPrChannel(), samples);

            size_t idx = 0;
            for (size_t i = filename.size(); i > 0; --i) {
                if (filename[i-1] == '/') {
                    idx = i;
                    break;
                }
            }

            std::string tmpDir = ::getTmpDir();
            tmpss << tmpDir << filename.substr(idx, std::string::npos) << ".wav";

            std::stringstream ss;
            ss << "ffmpeg -i " << filename << " -ar 5512 " << tmpss.str();
            FILE* cmd = popen(ss.str().c_str(), "r");
            int res = pclose(cmd);
            if (res == -1) {
                std::stringstream rmss;
                rmss << "rm -rf " << tmpss.str();
                cmd = popen(rmss.str().c_str(), "r");
                res = pclose(cmd);
                return; // error
            }
            std::stringstream rmss;
            rmss << "rm -rf " << tmpss.str();
            cmd = popen(rmss.str().c_str(), "r");
            int resCmd = pclose(cmd);            
        } else {
            AudioFile a(filename.c_str());
            AudioStream as(a.getStream(0));
            as.read(a.getNumberOfSamplesPrChannel(), samples);

        }
        std::cout << samples.size() << std::endl;
        for (size_t i = 0; i < samples.size(); ++i) {
            std::cout << samples[i] << std::endl;
        }
        // implement ismir.
        ::generateFingerprintStream(samples, res);
    }
    
    size_t 
    fingerprint_strategy_ismir::getFrameLength() { return ::frameLength; }

    size_t
    fingerprint_strategy_ismir::getAdvance() { return ::advance; }

    size_t
    fingerprint_strategy_ismir::getSampleRate() { return ::sampleRate; }

}
