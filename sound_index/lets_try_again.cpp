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
struct FingerprintInfo {
    size_t fileId;  // always fits in 32 bits
    int64_t offset; // always fits in 32 bits.

    FingerprintInfo(size_t fileId, int64_t offset) : fileId(fileId), offset(offset) {};

};

map<size_t, vector<FingerprintInfo> > db;

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

    //vector<vector<complex<double> > > spectrogram;
    //computeSpectrogram(samples, spectrogram);

    // compute most significant feature in 5 frequency ranges
    // for all time slices.

    for (size_t timeSlice = 0; timeSlice < samples.size()/JUMP; ++timeSlice) {

	vector<complex<double> > spectrogram; computeSpectrogram(samples, spectrogram, timeSlice);
	FingerprintInfo info(fileId, timeSlice * JUMP);
	size_t fingerprint = computeFingerprint(spectrogram, timeSlice);
	if (db.count(fingerprint) > 0) {
	    db[fingerprint].push_back(info);
	} else {
	    db.insert(make_pair(fingerprint, vector<FingerprintInfo>(1, info)));
	}
	++fingerprintCounter;
    }
}

void writeDBToDisk(const char *filename) {
    uint32_t headerLength = 4; // bytes
    uint32_t numberOfFiles = static_cast<uint32_t>(files.size());
    for (size_t i = 0; i < numberOfFiles; ++i) {
	headerLength += files[i].size() + 4;
    }

    char buffer[1024*1024*2]; //2mb buffer for I/O
    size_t pos = 0;
    FILE * f = fopen(filename, "w");

    // write header.
    memcpy(buffer + pos, &headerLength, sizeof(uint32_t));
    pos += sizeof(uint32_t);
    memcpy(buffer + pos, &numberOfFiles, sizeof(uint32_t));
    pos += sizeof(uint32_t);
    for (size_t i = 0; i < numberOfFiles; ++i) {
	if (pos + files[i].size() + 4 >= 1024*1024*2) {
	    // flush
	    fwrite(buffer, 1, pos, f);
	    pos = 0;
	}
	uint32_t length = static_cast<uint32_t>(files[i].size());
	memcpy(buffer + pos, &length, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	const char *str = files[i].c_str();
	memcpy(buffer + pos, str, length);
	pos += length;
    }

    typedef map<size_t, vector<FingerprintInfo> >::iterator dbIter_t;

    for (dbIter_t it = db.begin(); it != db.end(); ++it) {
	for (size_t i = 0; i < it->second.size(); ++i) {
	    // Fingerprintinfo requires 64 bits = 8 bytes
	    // the fingerprint requires 32 bits = 4 bytes
	    if (pos + 8 + 4 >= 1024*1024*2) {
		// flush
		fwrite(buffer, 1, pos, f);
		pos = 0;
	    }
	    uint32_t fp = it->first;
	    uint64_t info = it->second[i].fileId;

	    info = info << 32;
	    info += it->second[i].offset;
	    memcpy(buffer + pos, &(fp), sizeof(fp));
	    pos += sizeof(fp);
	    memcpy(buffer + pos, &(info), sizeof(info));
	    pos += sizeof(info);
	}
	
    }

    if (pos > 0) {
	// flush
	fwrite(buffer, 1, pos, f);
    }

    fclose(f);
}

void loadFromDisk(const char *filename) {
    FILE *f = fopen(filename, "r");

    size_t read = 0;

    // first we parse the header information.
    // read first 4 bytes to get the length of the header.
    uint32_t headerLength = 0;
    read = fread(&headerLength, 4, 1, f);
    if (read < 4) {
	// error handling.
    }

    char * header = new char[headerLength];
    size_t headerPos = 0;
    read = fread(header, 1, headerLength, f);
    if (read < headerLength) {
	// error handling. File is not in correct format or missing pieces.
    }

    uint32_t numberOfFiles;
    memcpy(&numberOfFiles, header, sizeof(uint32_t));
    headerPos += sizeof(uint32_t);

    for (size_t i = 0; i < numberOfFiles; ++i) {
	uint32_t filenameLength = 0;
	memcpy(&filenameLength, header + headerPos, sizeof(uint32_t));
	headerPos += sizeof(uint32_t);
	auto_ptr<char> aFilename(new char[filenameLength]);
	char * filename = aFilename.get();
	memcpy(filename, header + headerPos, filenameLength);
	headerPos += filenameLength;
	files.push_back(std::string(filename));

    }

    delete[] header;

    size_t sizeOfElmts = 4 + 8; // bytes: 4 for fingerprint, 8 for 'fileId+offset'.

    char * buffer = new char[1024*1024*sizeOfElmts];
    size_t pos = 0;

    do {
	read = fread(buffer, sizeOfElmts, 1024*1024, f);
	for (size_t i = 0; i < read; ++i) {
	    uint64_t fingerprint;
	    size_t index;
	    memcpy(&fingerprint, buffer + i*sizeOfElmts, 4);
	    memcpy(&index, buffer + i*sizeOfElmts + 4, 8);
	    size_t fileId = index >> 32;
	    size_t offset = index & 0xFFFFFFFF;
	    
	    FingerprintInfo info(fileId, offset);

	    if (db.count(fingerprint) > 0) {
		db[fingerprint].push_back(info);
	    } else {
		db.insert(make_pair(fingerprint, vector<FingerprintInfo>(1, info)));
	    }
	    ++fingerprintCounter;

	}
    } while (read == 1024*1024);

    fclose(f);
    delete[] buffer;

}

int main(int argc, char *argv[]) {
    timeval dbBuildStart, dbBuildEnd, queryStart, queryEnd;
    size_t elapsedSeconds;
/*
    size_t numDBFiles = atoi(argv[1]);

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
    double ratio = static_cast<double>(fingerprintCounter)/db.size();
    cout << "Time to build db: " << elapsedSeconds << " seconds" << endl;
    cout << "Unique fingerprints: " << db.size() << endl;
    cout << "Total fingerprints: " << fingerprintCounter << endl;
    cout << "Ratio: " << fingerprintCounter << " / " << db.size() << " = " << ratio << endl;
    writeDBToDisk("test.out");
*/
    loadFromDisk("test.out");
    //writeDBToDisk("test2.out");
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
