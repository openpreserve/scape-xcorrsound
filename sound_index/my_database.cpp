#include <vector>
#include <map>
#include <iostream>
#include <cstdio>
#include <string.h>

#include "my_database.h"
#include "my_fingerprint.h"
#include "../AudioFile.h"

using namespace std;

namespace {
    map<my_fingerprint, vector<size_t> > db;
    vector<string> files;

    inline
    void printInt(size_t input) {
	for (size_t i = 0; i < 36; ++i) {
	    std::cout << ((input >> (35-i)) & 1);
	}
	std::cout << std::endl;
    }

    size_t maxInOneBucket = 0;
    size_t numberOfFingerPrints = 0;
    size_t numberOfBuckets = 0;
    size_t theMaxBucket = 0;
    size_t collisions = 0;
    size_t yetAnotherCounter = 0;
    
    size_t spaceUsage = 0; //meassured in bytes.

}

void my_database::printStatistics() {
    double avgBucketSize = static_cast<double>(numberOfFingerPrints) / static_cast<double>(numberOfBuckets);

    std::cout << "Number of buckets: " << numberOfBuckets << std::endl;
    std::cout << "Collisions: " << collisions << std::endl;
    std::cout << "maxInOneBucket: " << maxInOneBucket << std::endl;
    std::cout << "average bucket size: " << avgBucketSize << std::endl;
    std::cout << "# fingerprints: " << numberOfFingerPrints << std::endl;
    std::cout << "the max bucket: "; printInt(theMaxBucket);
    std::cout << "the max bucket: " << theMaxBucket << std::endl;
    std::cout << "space usage: " << spaceUsage << " bytes" << std::endl;
}

vector<size_t> my_database::retrieveIndices(const my_fingerprint &query) {
    if (db.count(query) != 0) {
	vector<size_t> ret = db[query];
	return ret;
    } else {
	vector<size_t> ret;
	return ret;
    }
}

void my_database::insert_file(const char *fileName) {
    AudioFile a(fileName);
    vector<int16_t> samples;
    a.getSamplesForChannel(0, samples);

    for (size_t i = 0; i+2048 < samples.size(); i+= 200) {

	bool flag = false;
	if (i+200+2048 >= samples.size()) flag = true;

	my_fingerprint myf(samples.begin() + i, samples.begin() + i + 2048, flag);
	    
	size_t index = (files.size() << 40) + i;

	this->insert(myf, index);
    }

    files.push_back(string(fileName));
}

void my_database::insert(const my_fingerprint &f, const size_t index) {

    if (f.getPrint() == 60129542144LL) {
	++yetAnotherCounter;
    }
    spaceUsage += sizeof(index) + sizeof(f);

    // just internal memory atm.
    ++numberOfFingerPrints;
    size_t dbCnt = db.count(f);
    if (dbCnt != 0) {
	++collisions;
	db[f].push_back(index);
	if (maxInOneBucket < db[f].size()) {
	    maxInOneBucket = db[f].size();
	    theMaxBucket = f.getPrint();
	}
    } else {
	++numberOfBuckets;
	db.insert(pair<my_fingerprint, vector<size_t> >(f, vector<size_t>(1, index)));
	if (maxInOneBucket == 0) {
	    maxInOneBucket = 1;
	}
    }
}

vector<int16_t> my_database::getSamplesFromIndex(index_t index) {
    vector<int16_t> res;

    size_t fileIdx = index >> 40;
    size_t offsetBegin = index - (fileIdx<<40);

    AudioFile a(files[fileIdx].c_str());
    
    a.getSamplesForChannelInRange(offsetBegin, offsetBegin+2048, res);

    return res;
}

void
my_database::writeToDisk(const char * filename) {
    // file will look like this:
    // [header data]
    // header -> length of header 32 bits,
    //           number of files used
    //           [file list]
    // file list -> length of file name 32 bits,
    //              filename
    //              [file list]
    // OR
    // file list -> empty
    // data -> data_block data
    // data -> empty
    // data_block -> 8 bytes finger print,
    //               8 bytes index,
    //               [data]

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

    typedef map<my_fingerprint, vector<size_t> >::iterator map_it;

    for (map_it iter = db.begin(); iter != db.end(); ++iter) {
	for (size_t i = 0; i < iter->second.size(); ++i) {

	    if (pos + sizeof(iter->second[i]) + sizeof(iter->first) >= 1024*1024*2) {
		// flush
		fwrite(buffer, 1, pos, f);
		pos = 0;
	    }

	    memcpy(buffer + pos, &(iter->first), sizeof(iter->first));
	    pos += sizeof(iter->first);
	    memcpy(buffer + pos, &(iter->second[i]), sizeof(iter->second[i]));
	    pos += sizeof(iter->second[i]);
	}
    }

    if (pos > 0) {
	// flush
	fwrite(buffer, 1, pos, f);
    }

    fclose(f);

}

void
my_database::loadFromDisk(const char * filename) {

    size_t sizeOfElmts = sizeof(my_fingerprint) + sizeof(size_t);
    char * buffer = new char[1024*1024*sizeOfElmts];
    size_t pos = 0;
    FILE *f = fopen(filename, "r");

    size_t read;
    do {
	read = fread(buffer, sizeOfElmts, 1024*1024, f);
	for (size_t i = 0; i < read; ++i) {
	    uint64_t fingerprint;
	    size_t index;
	    memcpy(&fingerprint, buffer + i*sizeOfElmts, sizeof(fingerprint));
	    memcpy(&index, buffer + i*sizeOfElmts + sizeof(my_fingerprint), sizeof(index));
	    my_fingerprint myf(fingerprint);
	    insert(myf, index);
	}
    } while (read == 1024*1024);

    files.push_back("/home/jasn/Desktop/mp3test/P3_18_20.wav");
    fclose(f);
}
