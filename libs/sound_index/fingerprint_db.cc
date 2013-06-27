#include <AudioFile.h>
#include <fingerprint_db.hh>
#include <fingerprint_stream.hh>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

namespace si = sound_index;

namespace {

    void writeDBToDisk(std::string dbFilename, 
		       std::vector<uint32_t> &db,
		       std::string audioFilename) {

	std::ofstream of(dbFilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	size_t init = of.tellp() / 4;
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
/*
    void readDBFromDisk(std::string filename, std::vector<uint32_t> &out) {
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	out.clear();

	uint32_t buffer[(1<<20)];

	do {
	    in.read((char*) buffer,(1<<20) * sizeof(uint32_t));
	    for (size_t i = 0; i < in.gcount()/sizeof(uint32_t); ++i) {
		out.push_back(buffer[i]);
	    }

	} while (!in.eof());

	in.close();

	std::string mapFilename = "map_" + filename;
	std::ifstream in_map(mapFilename.c_str(), std::ios::in);
	while (!in_map.eof()) {
	    size_t idx; std::string filename;
	    in_map >> idx >> filename;
	    files.insert(make_pair(idx, filename));
	}
	in_map.close();
    }
*/

/*
    void insert_file(const char *file) {

	AudioFile a(file);
    
	std::vector<int16_t> samples;

	a.getSamplesForChannel(0, samples);

	std::vector<uint32_t> fingerprints;

	generateFingerprintStream(samples, fingerprints);

	db.insert(db.end(), fingerprints.begin(), fingerprints.end());

	files.insert(make_pair(db.size(), std::string(file)));
    
    }
*/
}

void si::fingerprint_db::open(std::string filename) {
    this->dbFilename = filename;
}

void si::fingerprint_db::insert(std::string filename) {
    // assume we can have entire filename in memory.
    // filename is the filename of a wav file.

    AudioFile a(filename.c_str());
    
    std::vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    std::vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);
    
    // append fingerprint stream to this->dbFilename
    writeDBToDisk((this->dbFilename).c_str(), fingerprints, filename);
    
}

std::vector<std::string> si::fingerprint_db::query_scan(std::string filename) {

}
    
void si::fingerprint_db::merge(fingerprint_db &other) {

}

void si::fingerprint_db::close() {

}
