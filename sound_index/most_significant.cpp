#include <complex>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include "stdint.h"

#include "../AudioFile.h"
#include "computeFFT.h"

#include <sys/time.h>

using namespace std;

// constants
const float sampleRate = 5512.5;
const size_t sampleSize = 2048;
const float hz = sampleRate/sampleSize;

struct my_hasher {
    size_t operator() (const pair<double, double> &key) const {
	size_t first = key.first*100;
	size_t second = key.second*100;
	
	size_t a = 32277176;
	size_t p = (1LL<<31)-1;

	return (first * second * a) % p;

    }
};

unordered_map<pair<double, double>, size_t, my_hasher> db;

void printUsage() {

}

double getMostSignificantFrequency(vector<int16_t>::iterator begin, vector<int16_t>::iterator end) {
    vector<int16_t> samples(begin, end);

    vector<complex<double> > transform(samples.size());

    computeFFT(samples, transform);

    double mostSignificantFrequency = 0.0;
    double mostSignificantAmplitude = 0.0;

    for (size_t i = 0; i < transform.size(); ++i) {
	double frequency = i * hz;
	if (frequency >= sampleRate/2) { // can't observe higher frequencies (thx Shannon)
	    break;
	}
	if (frequency < 220.0) { // want frequency at least this high.
	    continue;
	}

	double amplitude = transform[i].real()*transform[i].real() + transform[i].imag()*transform[i].imag();
	if (amplitude > mostSignificantAmplitude) {
	    mostSignificantAmplitude = amplitude;
	    mostSignificantFrequency = frequency;
	}
    }
    
    return mostSignificantFrequency;

}

void insert_file(char *filename, size_t fileNumber) {
    AudioFile a(filename);
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    size_t jump = 128;
    size_t fpCount = sampleRate/jump; //how many fingerprints we build concurrently
    
    vector<pair<double, double> > fingerprints(fpCount);

    if (samples.size() <= 2*sampleRate) return; // need at least two seconds.

    for (size_t i = 0; i < samples.size(); i+=jump) {
	size_t fingerprintsIdx = (i % sampleSize) / jump;
	fingerprints[fingerprintsIdx].first = fingerprints[fingerprintsIdx].second;
	fingerprints[fingerprintsIdx].second = getMostSignificantFrequency(samples.begin()+i, samples.begin()+i+sampleSize);
	
	if (i/jump >= fpCount) {
	    size_t idx = (fileNumber << 40);
	    idx += i-sampleSize;
	    db.insert(make_pair(fingerprints[(i%sampleSize)/128], idx));
	}

    }

}

void query(const char *filename) {
    AudioFile a(filename);
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    // start one second in;
    size_t start = sampleRate;
    size_t jump = 128;
    size_t fpCount = 16; //how many fingerprints we build concurrently

    if (samples.size() <= 2*sampleRate) return; // need at least two seconds.

    for (size_t i = 0; i < jump; i += jump/fpCount) {
	double ms1 = getMostSignificantFrequency(samples.begin()+start+i, samples.begin()+start+i+sampleSize);
	double ms2 = getMostSignificantFrequency(samples.begin()+start+i+sampleSize, samples.begin()+start+i+sampleSize + sampleSize);

	pair<double, double> key(ms1, ms2);

	if (db.count(key) != 0) {

	    size_t fileNumber = db[key]>>40;
	    size_t samplestamp = db[key] & ((1LL<<40)-1);
	    
	    size_t seconds = samplestamp / sampleRate;
	    size_t hours = seconds / 3600;
	    size_t minutes = (seconds%3600) / 60;
	    seconds = seconds - 3600*hours - 60*minutes;

	    cout << fileNumber << ": ";	    
	    if (hours < 10) {
		cout << "0";
	    }
	    cout << hours << ":";
	    if (minutes < 10) {
		cout << "0";
	    }
	    cout << minutes << ":";
	    if (seconds < 10) {
		cout << "0";
	    }
	    cout << seconds << endl;
	}
	
    }
    
    vector<pair<double, double> > fingerprints(fpCount);

    if (samples.size() <= 2*sampleRate) return; // need at least two seconds.

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
	//std::cout << "Wrong number of arguments, expected at least 1" << std::endl;
	printUsage();
	return 1;
    }

    int numDBFiles = atoi(argv[1]);

    timeval dbBuildStart, dbBuildEnd, queryStart, queryEnd;
    gettimeofday(&dbBuildStart, NULL);

    for (size_t arg = 2; arg < numDBFiles+2; ++arg) {
	cout << "fisk" << arg << endl;
    	insert_file(argv[arg], arg-1);
    }
    gettimeofday(&dbBuildEnd, NULL);
    cout << "fisk" << endl;
    // REPL

    while (1) {
	string x;
	cout << "Enter a query file: ";
	cin >> x;
	
	query(x.c_str());
	
    }


}
