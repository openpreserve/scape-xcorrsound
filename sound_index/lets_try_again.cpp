#include <algorithm>
#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "stdint.h"
#include <sys/time.h>

#include "../AudioFile.h"
#include "computeFFT.h"
#include "db_wrapper.h"
#include "FingerprintInfo.h"

using namespace std;

vector<string> files;

size_t fingerprintCounter = 0;

//size_t FRAME_SZ = 512;
size_t FRAME_SZ = 4096;
size_t JUMP = 512;
//size_t FREQUENCY_RANGES[] = {20, 40, 65, 100}; // should be 95.
//size_t FREQUENCY_RANGES[] = {256, 512, 768, 1024};
size_t FREQUENCY_RANGES[] = {512, 1024, 1536, 2048};
//                        215   430  700  1076
//                         344  689  1033  1378  

//map<size_t, vector<FingerprintInfo> > db;
char dbFile[] = "/media/Big_disk/index_files/test.db";
db_wrapper db(dbFile);

size_t timeDiff(timeval &tvStart, timeval &tvEnd) {
    size_t elapsedMS = (tvEnd.tv_usec + tvEnd.tv_sec*1000000) - (tvStart.tv_usec + tvStart.tv_sec*1000000);
    return elapsedMS;
}

void computeSpectrogram(vector<int16_t> &samples, vector<complex<double> > &spectrogram, size_t timeSlice) {
    spectrogram.clear();

    size_t start = timeSlice * JUMP;
    vector<int16_t> input(FRAME_SZ, 0);
    for (size_t i = start; i < start + FRAME_SZ; ++i) {
	input[i-start] = samples[i];
    }
    computeFFT(input, spectrogram);
}

size_t getFingerprint(vector<size_t> &max_freqs) {
    size_t fp = 0;
    // each bucket needs 8 bits
    // freq i and i+1 are considered the same if i%2==0.
    // this is only for reducing bit usage.

    for (size_t i = 0; i < max_freqs.size(); ++i) {
	//fp += ((max_freqs[i] - (max_freqs[i]%2))>>1); // should be modulo 4
	if (i > 0) fp = fp << 8;

	fp += ((max_freqs[i] & 511)>>1);
    }
    if (fp > (1LL<<32)) cout << fp << endl;
    return fp;
}

bool fileIdComparer(const pair<FingerprintInfo,size_t> &lhs, const pair<FingerprintInfo,size_t> &rhs) {
    return lhs.first.fileId < rhs.first.fileId;
}

size_t computeFingerprint(vector<complex<double> > &spectrogram, size_t timeSlice) {
    size_t bucket = 0;
    vector<double> max_amplitude(4, 0);
    vector<size_t> max_frequency(4, 0);
    for (size_t freq = 0; freq < FRAME_SZ; ++freq) {
	
	if (abs(spectrogram[freq]) > max_amplitude[bucket]) {
	    max_amplitude[bucket] = abs(spectrogram[freq]);
	    max_frequency[bucket] = freq; 
	}

	for (size_t i = 1; i < 4; ++i) {
	    if (freq >= FREQUENCY_RANGES[i-1] && freq <= FREQUENCY_RANGES[i]) {
		bucket = i;
	    }
	}

    }
    //cout << max_frequency[3] << endl;
    return getFingerprint(max_frequency);
}

vector<FingerprintInfo> query(const string filename) {
    AudioFile a(filename.c_str());
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    //vector<vector<complex<double> > > spectrogram;
    //computeSpectrogram(samples, spectrogram);

    vector<pair<FingerprintInfo, size_t> > results;
    set<size_t> fileIds;
    for (size_t timeSlice = 0; timeSlice < samples.size() / JUMP; ++timeSlice) {
	vector<complex<double> > spectrogram; computeSpectrogram(samples, spectrogram, timeSlice);
	size_t fp = computeFingerprint(spectrogram, timeSlice);
	vector<FingerprintInfo> tmp;
	db.query(fp, tmp);
	for (size_t i = 0; i < tmp.size(); ++i) {		
	    results.push_back(make_pair(tmp[i], timeSlice*JUMP));
	    fileIds.insert(tmp[i].fileId);
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

    //vector<vector<complex<double> > > spectrogram;
    //computeSpectrogram(samples, spectrogram);

    // compute most significant feature in 4 frequency ranges
    // for all time slices.
    
    vector<pair<size_t, FingerprintInfo> > data;
    fingerprintCounter += samples.size() / JUMP;
    for (size_t timeSlice = 0; timeSlice < samples.size()/JUMP; ++timeSlice) {
	vector<complex<double> > spectrogram; computeSpectrogram(samples, spectrogram, timeSlice);
	FingerprintInfo info(fileId, timeSlice * JUMP);
	size_t fingerprint = computeFingerprint(spectrogram, timeSlice);
	data.push_back(make_pair(fingerprint, info));
    }
    db.bulk_insert(data);
}

int main(int argc, char *argv[]) {
    timeval dbBuildStart, dbBuildEnd, queryStart, queryEnd;
    size_t elapsedSeconds;

    size_t numDBFiles = atoi(argv[1]);
/*
    gettimeofday(&dbBuildStart, NULL);

    for (size_t arg = 2; arg < numDBFiles+2; ++arg) {
	char *x = argv[arg];
	cout << "Processing file " << arg-1 << " ... " << flush;
    	insert_file(x, arg-1);
	files.push_back(string(x));
	cout << "Done" << endl;
    }
    gettimeofday(&dbBuildEnd, NULL);
    elapsedSeconds = timeDiff(dbBuildStart, dbBuildEnd) / 1000000;
    //double ratio = static_cast<double>(fingerprintCounter)/db.size();
    cout << "Time to build db: " << elapsedSeconds << " seconds" << endl;
    //cout << "Unique fingerprints: " << db.size() << endl;
    cout << "Total fingerprints: " << fingerprintCounter << endl;
    //cout << "Ratio: " << fingerprintCounter << " / " << db.size() << " = " << ratio << endl;
    //writeDBToDisk("test.out");

*/
    //cout << db.size() << endl;
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
