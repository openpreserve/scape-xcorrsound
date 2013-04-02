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
const size_t frames = 3;

struct my_hasher {
    size_t operator() (const pair<double, double> &key) const {
	size_t first = key.first*100;
	size_t second = key.second*100;
	
	size_t a = 32277176;
	size_t p = (1LL<<31)-1;

	return (first * second * a) % p;

    }
};

struct another_hasher {
    size_t operator() (const double *key) const {
	size_t first = key[0] * 100;
	size_t second = key[1] * 100;
	size_t a = 32277176;
	size_t p = (1LL<<31)-1;
	
	return 	(first * second * a) % p;

    }
};

struct my_comp {
    bool operator()(const double *a, const double *b) const {
	if (a[0] < b[0]) return true;
	if (a[0] == b[0] && a[1] < b[1]) return true;
	if (frames >= 3) {
	    if (a[0] == b[0] && a[1] == b[1] && a[2] < b[2]) return true;
	}
	return false;
    }

};

//unordered_map<pair<double, double>, size_t, my_hasher> db;

//unordered_map<size_t, size_t> db;
//unordered_map<double*, size_t, another_hasher> db;

map<double*, size_t, my_comp> db;

void printUsage() {
    cout << "hello" << endl;
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

size_t getFingerprint(vector<int16_t> &samples, size_t start) {
    size_t freqs[frames] = {0,0};
    for (size_t j = 0; j < frames; ++j) {
	freqs[j] = static_cast<int>(100*getMostSignificantFrequency(samples.begin()+start+j*sampleRate, samples.begin()+start+(j+1)*sampleRate))/100;
    }
    //freqs[2] = 0;
    size_t fingerprint = (freqs[0]<<24)+(freqs[1]<<12)+(freqs[2]); // generalise this line
    fingerprint = freqs[0]*freqs[1];
    return fingerprint;
}

void insert_file(const char *filename, size_t fileNumber) {

    AudioFile a(filename);
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    size_t jump = 128;
    size_t fpCount = sampleRate/jump; //how many fingerprints we build concurrently
    
    vector<double*> fingerprints(fpCount);

    if (samples.size() <= frames*sampleRate) return; // need at least two seconds.

    for (size_t i = 0; i < samples.size(); i+=jump) {

	size_t fingerprintsIdx = (i % sampleSize) / jump;
	if (i < sampleSize) fingerprints[fingerprintsIdx] = new double[frames];
	for (size_t j = 0; j < frames-1; ++j) {
	    fingerprints[fingerprintsIdx][j] = fingerprints[fingerprintsIdx][j+1];
	}
	fingerprints[fingerprintsIdx][frames-1] = floor(getMostSignificantFrequency(samples.begin()+i, samples.begin()+i+sampleSize));

	if (i >= (frames-1)*sampleSize) {

	    size_t idx = (fileNumber << 40);
	    idx += i-(frames-1)*sampleSize;
	    size_t freqs[frames];
	    for (size_t j = 0; j < frames; ++j) {
		freqs[j] = static_cast<size_t>(fingerprints[fingerprintsIdx][j]*100)/100;
	    }

	    //size_t fingerprint = (freqs[0]<<24)+(freqs[1]<<12)+(freqs[2]);
	    //fingerprint = freqs[0]*freqs[1];
	    //db.insert(make_pair(fingerprint, idx));

	    // if (i-2*2048 >= 5842720 && i-2*2048 <= 5881304) {
	    // 	cout << fingerprints[fingerprintsIdx][0] << " , " << fingerprints[fingerprintsIdx][1] << " , " << fingerprints[fingerprintsIdx][2] << " : " << fingerprints[fingerprintsIdx] << endl;
	    // }
	    double *tmp = new double[frames];
	    tmp[0] = fingerprints[fingerprintsIdx][0];
	    tmp[1] = fingerprints[fingerprintsIdx][1];
	    db.insert(make_pair(tmp, idx));
	}

    }

}

void query(const char *filename) {

    AudioFile a(filename);
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    // start one second in;
    size_t start = sampleRate;
    size_t jump = 448;
    size_t fpCount = 32; //how many fingerprints we build

    if (samples.size() <= frames*sampleRate) return; // need at least <frames> points.

    for (size_t i = 0; i < jump*fpCount; i += jump) {
	//size_t fingerprint = getFingerprint(samples, i);
	double fingerprint[frames];
	for (size_t j = 0; j < frames; ++j) {
	    //fingerprint[j] = static_cast<int>(100*getMostSignificantFrequency(samples.begin()+start+j*sampleRate, samples.begin()+start+(j+1)*sampleRate))/100;
	    fingerprint[j] = floor(getMostSignificantFrequency(samples.begin()+start+i+j*sampleSize, samples.begin()+start+i+(j+1)*sampleSize));
	}
	cout << fingerprint[0] << " , " << fingerprint[1] << " , " << fingerprint[2] << endl;
	if (db.count(fingerprint) != 0) {

	    size_t fileNumber = db[fingerprint]>>40;
	    size_t samplestamp = db[fingerprint] & ((1LL<<40)-1);
	    
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
    
}

int main(int argc, char *argv[]) {

    size_t numDBFiles = atoi(argv[1]);

    timeval dbBuildStart, dbBuildEnd, queryStart, queryEnd;
    gettimeofday(&dbBuildStart, NULL);

    for (size_t arg = 2; arg < numDBFiles+2; ++arg) {
	char *x = argv[arg];
	cout << "fisk" << arg << endl;
    	insert_file(x, arg-1);
    }
    gettimeofday(&dbBuildEnd, NULL);
    size_t elapsedSeconds = (dbBuildEnd.tv_usec + dbBuildEnd.tv_sec*1000000) - (dbBuildStart.tv_usec + dbBuildStart.tv_sec*1000000);
    elapsedSeconds /= 1000000;
    cout << "Time to build db: " << elapsedSeconds << " seconds" << endl;

    // REPL
    while (1) {
	string x;
	cout << "Enter a query file: ";
	cin >> x;
	if (x.size() < 3) continue;
	query(x.c_str());
	
    }
}
