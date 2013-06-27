#include <AudioFile.h>
#include <fingerprint_db.hh>
#include <fingerprint_stream.hh>
#include <fstream>
#include <hamming.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

namespace si = sound_index;

namespace {

    uint32_t hammingLookup16(uint32_t n) {
	uint16_t a = n & 0xFFFF;
	uint16_t b = n>>16;

	return hamming16[a] + hamming16[b];
    }

    void writeDBToDisk(std::string dbFilename, 
		       std::vector<uint32_t> &db,
		       std::string audioFilename) {

	std::ofstream of(dbFilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	//size_t init = of.tellp() / 4;
	for (size_t i = 0; i < db.size(); ++i) {
	    of.write((char*) &db[i], sizeof(uint32_t));
	}
	size_t end = of.tellp() / 4;
	of.close();

	std::string mapFilename = "map_" + dbFilename;
	std::ofstream mof(mapFilename.c_str(), std::ios::out | std::ios::app);

	// typedef std::map<size_t, string>::iterator m_iter;
	// for (m_iter iter = files.begin(); iter != files.end(); ++iter) {
	//     mof << iter->first + init << " " << iter->second << endl;
	// }

	mof << end << " " << audioFilename << std::endl;
	mof.close();

    }

}

void si::fingerprint_db::open(std::string filename) {

    size_t idx = 0;
    for (size_t i = filename.size(); i > 0; --i) {
	if (filename[i-1] == '/') {
	    idx = i;
	    break;
	}
    }
    
    std::string mapFilePrefix = filename.substr(0, idx);
    std::string mapFileSuffix = filename.substr(idx, std::string::npos);
    std::stringstream ss;
    ss << mapFilePrefix << "map_" << mapFileSuffix;

    std::string mapFile(ss.str());


    this->dbFilename = filename;
    
    std::ifstream fin(mapFile.c_str(), std::ifstream::in);

    size_t id; std::string file;

    while (fin >> id >> file) {
	idToFile[id] = file;
    }

}

void si::fingerprint_db::insert(std::string filename) {
    // assume we can have the entire file "filename" in memory.
    // "filename" is the filename of a wav file.

    AudioFile a(filename.c_str());
    
    std::vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    std::vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);
    
    // append fingerprint stream to this->dbFilename
    writeDBToDisk((this->dbFilename).c_str(), fingerprints, filename);
    
}

void si::fingerprint_db::query_scan(std::string filename, std::vector<std::string> &ret) {
    // "filename" is the name of the wav file that is our query.
    ret.clear();
    AudioFile a(filename.c_str());
    
    std::vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    std::vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);

    // size_t bestMatch = 1024*1024;
    // size_t bestMatchDistance = fingerprints.size();

    std::vector<uint32_t> db(1024*1024+32);
    
    std::ifstream fin(this->dbFilename.c_str(), std::ifstream::in | std::ifstream::binary);
    
    uint32_t *buf = new uint32_t[1024*1024];

    while (fin) {
	for (size_t i = 0; i < 32; ++i) {
	    db[i] = db[db.size()-32+i];
	}

	fin.read((char*)buf, 1024*1024*sizeof(uint32_t));

	size_t count = fin.gcount()/sizeof(uint32_t);
	
	for (size_t i = 0; i < count; ++i) {
	    db[i+32] = buf[i];
	}


	for (size_t i = 1; i < db.size()-32; ++i) {
	    size_t dist = 0;
	    for (size_t j = fingerprints.size()/2; j < fingerprints.size()/2+32; ++j) {
		uint32_t x = static_cast<uint32_t>(fingerprints[j] ^ db[i+j]);
		uint32_t cnt = hammingLookup16(x);
		dist += cnt;
	    }
	    
	    if (dist < 358) { //0.35 * 1024 ~= 358
		
		std::map<size_t, std::string>::iterator iter = idToFile.lower_bound(i);
	    
		//size_t fileId = iter->first;
	    
		std::string filename = iter->second;

		size_t start = 0;
		if (iter != idToFile.begin()) {
		    --iter;
		    start = iter->first;
		}

		size_t sampleInFile = i - start;

		std::string timestamp = getTimestampFromSeconds(sampleInFile*advance / sampleRate);
		
		std::stringstream ss;

		ss << "match in " << filename 
		   << " at " << timestamp
		   << " with distance " << dist;
		
		ret.push_back(ss.str());
	    }
	}

    }

}

void si::fingerprint_db::merge(fingerprint_db &a, fingerprint_db &b) {

}

void si::fingerprint_db::close() {

}
