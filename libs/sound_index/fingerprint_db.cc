#include <AudioFile.h>
#include <fingerprint_db.hh>
#include <fingerprint_strategy.hh>
#include <fingerprint_stream.hh>
#include <fstream>
#include <hamming.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

namespace si = sound_index;

namespace {

    uint32_t macro_sz = 256;

    uint32_t hammingLookup16(uint32_t n) {
        uint16_t a = n & 0xFFFF;
        uint16_t b = n>>16;

        return hamming16[a] + hamming16[b];
    }

    void writeDBToDisk(std::string dbFilename, 
                       std::vector<uint32_t> &db,
                       std::string indexedName) {

        std::ofstream of(dbFilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);

        for (size_t i = 0; i < db.size(); ++i) {
            of.write((char*) &db[i], sizeof(uint32_t));
        }
        size_t end = of.tellp() / 4;
        of.close();

        std::string mapFilename = "map_" + dbFilename;
        std::ofstream mof(mapFilename.c_str(), std::ios::out | std::ios::app);

        mof << end << " " << indexedName << std::endl;
        mof.close();

    }

    void getMapFile(std::string &filename, std::string &mapFile) {
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
        mapFile = ss.str();
    }


}

si::fingerprint_db::fingerprint_db() {
    this->fp_strategy = new si::fingerprint_strategy_ismir();
    //this->fp_strategy = new si::fingerprint_strategy_chroma();
}

void si::fingerprint_db::open(std::string filename) {

    std::string mapFile;
    getMapFile(filename, mapFile);

    this->dbFilename = filename;
    
    std::ifstream fin(mapFile.c_str(), std::ifstream::in);

    size_t id; std::string file;

    while (fin >> id >> file) {
        idToFile[id] = file;
    }

}

void si::fingerprint_db::insert(std::string filename, std::string indexedName) {
    // assume we can have the entire file "filename" in memory.
    // "filename" is the filename of a wav file.

    //AudioFile a(filename.c_str());
    
    // std::vector<int16_t> samples;

    //a.getSamplesForChannel(0, samples);

    std::vector<uint32_t> fingerprints;

    //generateFingerprintStream(samples, fingerprints);
    
    this->fp_strategy->getFingerprintsForFile(filename, fingerprints);

    // append fingerprint stream to this->dbFilename
    writeDBToDisk((this->dbFilename).c_str(), fingerprints, filename == "-" ? indexedName : filename);

}

void si::fingerprint_db::query_scan(std::string filename, std::vector<std::string> &ret) {
    // "filename" is the name of the wav file that is our query.
    ret.clear();
    //AudioFile a(filename.c_str());
    
    //std::vector<int16_t> samples;

    //a.getSamplesForChannel(0, samples);

    std::vector<uint32_t> fingerprints;

    //generateFingerprintStream(samples, fingerprints);

    this->fp_strategy->getFingerprintsForFile(filename, fingerprints);

    // size_t bestMatch = 1024*1024;
    // size_t bestMatchDistance = fingerprints.size();

    std::vector<uint32_t> db(1024*1024+macro_sz, 0);
    
    std::ifstream fin(this->dbFilename.c_str(), std::ifstream::in | std::ifstream::binary);

    uint32_t *buf = new uint32_t[1024*1024];
    size_t end = 0;
    size_t pos = 0;
    size_t prevMatchPos = std::numeric_limits<size_t>::max();
    while (fin) {

        for (size_t i = 0; i < macro_sz; ++i) {
            db[i] = db[db.size()-macro_sz+i];
        }

        fin.read((char*)buf, 1024*1024*sizeof(uint32_t));

        size_t count = fin.gcount()/sizeof(uint32_t);
        end = count + macro_sz;

        for (size_t i = 0; i < count; ++i) {
            db[i+macro_sz] = buf[i];
        }

        for (size_t i = 0; i < end-macro_sz; ++i,++pos) {
            if (pos - prevMatchPos < (this->fp_strategy->getSampleRate()) &&
                prevMatchPos != std::numeric_limits<size_t>::max()) continue;
            size_t dist = 0;
            // bool atLeastOne = false;
            size_t startJ = 50;
            for (size_t j = startJ; j < startJ+macro_sz; ++j) {
                uint32_t x = fingerprints[j] ^ db[i+j-startJ];
                //uint32_t cnt = hammingLookup16(x);
                uint32_t cnt = __builtin_popcountll(x);
                dist += cnt;
                // if (cnt <= 0) {
                //     atLeastOne = true;
                // }
            }

            if (dist < macro_sz*sizeof(uint32_t)*0.35*8) {
                prevMatchPos = pos;
                std::map<size_t, std::string>::iterator iter = idToFile.lower_bound(pos);

                std::string filenameRes = iter->second;

                size_t start = 0;
                if (iter != idToFile.begin()) {
                    --iter;
                    start = iter->first;
                }

                size_t sampleInFile = pos - start;
                size_t advance = this->fp_strategy->getAdvance();
                size_t sampleRate = this->fp_strategy->getSampleRate();

                std::string timestamp = getTimestampFromSeconds(sampleInFile*advance / sampleRate);
                //std::stringstream ss2; ss2 << (sampleInFile*advance/sampleRate) << "  i: " << i;
                //std::string timestamp(ss2.str());
                std::stringstream ss;
                
                ss << "match in " << filenameRes 
                   << " at " << timestamp
                   << " with distance " << dist;
                // if (atLeastOne) {
                //     ss << " and at least one perfect match";
                // }
                //ss << "fdsa" << std::endl;
                ret.push_back(ss.str());
            }
        }
    }
}

void si::fingerprint_db::query(std::string filename, std::vector<std::string> &results) {
    return;
}

void si::fingerprint_db::merge(std::vector<std::string> &inputs) {
    
    /**
     * Algorithm:
     *  for each db' in input
     *      let sz be db string length
     *      add sz to all entries in map_db'
     *      append db' string to db string
     *      append map_db' to map_db
     */
    std::string _mapFile;
    std::string _strFile(this->dbFilename);
    getMapFile(_strFile, _mapFile);

    std::ofstream _mapFileOut(_mapFile.c_str(), std::ofstream::out | std::ofstream::app);
    std::ofstream _strFileOut(_strFile.c_str(),
                              std::ofstream::out | 
                              std::ofstream::app | 
                              std::ofstream::binary);

    for (size_t i = 0; i < inputs.size(); ++i) {
        
        std::string mapFile;
        std::string strFile(inputs[i]);
        getMapFile(strFile, mapFile);
      
        size_t sz = _strFileOut.tellp()/4;
        
        std::ifstream strFileIn(strFile.c_str(), std::ofstream::binary);

        char c;
        while (strFileIn.get(c)) {
            _strFileOut.put(c);
            //strFileIn.read(buf, 1024*1024*16);
            //_strFileOut.write(buf, strFileIn.gcount());
        }

        std::ifstream mapFileIn(mapFile.c_str(), std::ifstream::in);
        size_t idx;
        std::string name;
            
        while (mapFileIn >> idx >> name) {
            _mapFileOut << idx+sz << " " << name << std::endl;
        }
        
    }

}

void si::fingerprint_db::close() {

}
