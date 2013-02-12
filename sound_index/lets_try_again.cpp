#include <algorithm>
#include <complex>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "stdint.h"
#include <sys/time.h>

#include "../AudioFile.h"
#include "computeFFT.h"

using namespace std;

size_t FRAME_SZ = 512;
size_t JUMP = 512;
size_t FREQUENCY_RANGES[] = {20, 40, 65, 100}; // should be 95.

struct FingerprintInfo {
    size_t fileId;
    int64_t offset;

    FingerprintInfo(size_t fileId, int64_t offset) : fileId(fileId), offset(offset) {};

};

unordered_map<size_t, vector<FingerprintInfo> > db;

size_t timeDiff(timeval &tvStart, timeval &tvEnd) {
    size_t elapsedMS = (tvEnd.tv_usec + tvEnd.tv_sec*1000000) - (tvStart.tv_usec + tvStart.tv_sec*1000000);
    return elapsedMS;
}

void computeSpectrogram(vector<int16_t> &samples, vector<vector<complex<double> > > &spectrogram) {
    spectrogram.clear();

    for (size_t i = 0; i < samples.size()-FRAME_SZ; i+=JUMP) {
	vector<int16_t> input(FRAME_SZ,0);
	for (size_t j = 0; j < FRAME_SZ; ++j) {
	    input[j] = samples[i+j];
	}
	vector<complex<double> > timeSlice;
	computeFFT(input, timeSlice);
	spectrogram.push_back(timeSlice);
    }
}

// void computeSpectrogram(vector<int16_t> &samples, vector<vector<complex<double> > > &spectrogram) {
//     spectrogram.clear();

//     for (size_t i = 0; i < samples.size() / FRAME_SZ; ++i) {
// 	vector<int16_t> input(FRAME_SZ,0);
// 	for (size_t j = 0; j < FRAME_SZ; ++j) {
// 	    input[j] = samples[i*FRAME_SZ+j];
// 	}
// 	vector<complex<double> > timeSlice;
// 	computeFFT(input, timeSlice);
// 	spectrogram.push_back(timeSlice);
//     }
// }

size_t getFingerprint(vector<size_t> &max_freqs) {
    size_t fp = 0;
    // each bucket needs 8 bit
    // freq i and i+1 are considered the same if i%2==0.
    // this is only for reducing bit usage.

    for (size_t i = 0; i < max_freqs.size(); ++i) {
	fp += ((max_freqs[i] - (max_freqs[i]%2))>>1); // should be modulo 4
	fp =  fp << 8;
    }

    return fp;
}

bool fileIdComparer(const pair<FingerprintInfo,size_t> &lhs, const pair<FingerprintInfo,size_t> &rhs) {
    return lhs.first.fileId < rhs.first.fileId;
}

size_t computeFingerprint(vector<vector<complex<double> > > &spectrogram, 
			  size_t timeSlice) {
    size_t bucket = 0;
    vector<double> max_amplitude(5, 0);
    vector<size_t> max_frequency(5, 0);
    for (size_t freq = 0; freq < FRAME_SZ; ++freq) {
	    
	if (abs(spectrogram[timeSlice][freq]) > max_amplitude[bucket]) {
	    max_amplitude[bucket] = abs(spectrogram[timeSlice][freq]);
	    max_frequency[bucket] = freq; 
	}

	for (size_t i = 1; i < 4; ++i) {
	    if (freq >= FREQUENCY_RANGES[i-1] && freq <= FREQUENCY_RANGES[i]) {
		bucket = i;
	    }
	}
    }

    return getFingerprint(max_frequency);
}

vector<FingerprintInfo> query(const string filename) {
    AudioFile a(filename.c_str());
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);  

    vector<vector<complex<double> > > spectrogram;
    computeSpectrogram(samples, spectrogram);

    vector<pair<FingerprintInfo, size_t> > results;
    set<size_t> fileIds;
    for (size_t timeSlice = 0; timeSlice < spectrogram.size(); ++timeSlice) {
	size_t fp = computeFingerprint(spectrogram, timeSlice);
	if (db.count(fp)) {
	    vector<FingerprintInfo> tmp(db[fp]);
	    for (size_t i = 0; i < tmp.size(); ++i) {		
		results.push_back(make_pair(tmp[i], timeSlice*JUMP));
		fileIds.insert(tmp[i].fileId);
	    }
	}
    }

    vector<FingerprintInfo> matches;

    sort(results.begin(), results.end(), fileIdComparer);

    size_t range = 0;
    for (set<size_t>::iterator fileIterator = fileIds.begin(); fileIterator != fileIds.end(); ++fileIterator) {
	size_t rangeStart = range;
	while (results[range].first.fileId == (*fileIterator)) ++range;
	++range;
	if (range-rangeStart < 4) continue; //magic constant
	
	// compute histogram.

	map<int64_t, size_t> timehits;
	for (size_t i = rangeStart; i < range; ++i) {
	    int64_t deltaTime = results[i].first.offset - results[i].second;
	    if (timehits.count(deltaTime)) {
		++timehits[deltaTime];
	    } else {
		timehits[deltaTime] = 1;
	    }
	}

	for (map<int64_t, size_t>::iterator it = timehits.begin(); it != timehits.end(); ++it) {
	    if (it->second >= 8) {
		// report a match.
		// negative offset means fingerprint matched very early in the db file
		
		if (matches.size() > 0) {
		    if (matches.back().fileId == *fileIterator && (abs(matches.back().offset - it->first)) < 5)
			continue;
		}
		matches.push_back(FingerprintInfo(*fileIterator, it->first));
	    }
	}
    }

    return matches;
}

void insert_file(const string filename, size_t fileId) {

    AudioFile a(filename.c_str());
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);  

    vector<vector<complex<double> > > spectrogram;
    computeSpectrogram(samples, spectrogram);

    // compute most significant feature in 5 frequency ranges
    // for all time slices.

    for (size_t timeSlice = 0; timeSlice < spectrogram.size(); ++timeSlice) {
	
	FingerprintInfo info(fileId, timeSlice * JUMP);
	size_t fingerprint = computeFingerprint(spectrogram, timeSlice);
	if (db.count(fingerprint) > 0) {
	    db[fingerprint].push_back(info);
	} else {
	    db.insert(make_pair(fingerprint, vector<FingerprintInfo>(1, info)));
	}

    }    
    cout << spectrogram.size() << endl;
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
    size_t elapsedSeconds = timeDiff(dbBuildStart, dbBuildEnd) / 1000000;
    cout << "Time to build db: " << elapsedSeconds << " seconds" << endl;
    cout << db.size() << endl;
    // REPL
    while (1) {
	string x;
	cout << "Enter a query file: ";
	cin >> x;
	if (x.size() < 3) continue;
	gettimeofday(&queryStart, NULL);
	vector<FingerprintInfo> matches(query(x.c_str()));
	gettimeofday(&queryEnd, NULL);
	elapsedSeconds = timeDiff(queryStart, queryEnd);
	if (matches.size() == 0) { cout << "no matches" << endl; continue; }
	cout << "query time: " << elapsedSeconds/1000 << " ms" << endl; 
	for (size_t i = 0; i < matches.size(); ++i) {
	    
	    cout << "File: " << matches[i].fileId << " at [in seconds] : " << matches[i].offset / 5512 << endl;
	}

    }
}
