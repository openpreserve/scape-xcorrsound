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
#include <memory>
#include <tuple>

namespace si = sound_index;

namespace {

    uint32_t macro_sz = 256;
    size_t fpSkip = 50; // skip this much into fingerprint array.
    size_t nearRange = 150; // how far to look around a position that
			    // is significantly different from noise.

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

    // percentage error. Break if in the 'noise zone'.
    // only check every now and then (i % 5)
    // and we must have a decent baseline, i.e. at least 10% through computation.
    // this is a heuristic to terminate early if we can see
    // there will not be a match here.
    std::pair<bool, int32_t> hammingEarlyTerminate(std::vector<uint32_t> &fingerprints, std::vector<uint32_t> &db, size_t start) {
	
	int32_t dist = 0;
	for (size_t i = 0 ; i < macro_sz; ++i) {

	    uint32_t x = fingerprints[i+fpSkip] ^ db[i+start];
	    uint32_t cnt = __builtin_popcountll(x);
	    dist += cnt;	    
	    
	    if ((i % 5) == 0 && i > macro_sz / 10) {
		double bitsSeenSoFar = i*sizeof(uint32_t)*8;
		double errorPercentage = dist / bitsSeenSoFar;
		if (errorPercentage > 0.43 && errorPercentage < 0.537) {
		    return std::make_pair(true, dist);
		}
	    }
	}
	return std::make_pair(false, dist);
    }

    int32_t fullHamming(std::vector<uint32_t> &fingerprints, std::vector<uint32_t> &db, size_t start) {

	int32_t dist = 0;
	for (size_t i = 0 ; i < macro_sz; ++i) {
	    uint32_t x = fingerprints[i+fpSkip] ^ db[i+start];
	    uint32_t cnt = __builtin_popcountll(x);
	    dist += cnt;	    
	}
	return dist;
	
    }

    std::pair<int32_t, size_t> fullCheck(std::vector<uint32_t> &fp, std::vector<uint32_t> &window) {

	int32_t bestDist = std::numeric_limits<int32_t>::max();
	size_t bestIdx = std::numeric_limits<size_t>::max();
	if (window.size() < macro_sz) return std::make_pair(bestDist, bestIdx);
	for (size_t i = 0; i < window.size() - macro_sz; ++i) {
	    int32_t dist = fullHamming(fp, window, i);
	    if (dist < bestDist) {
		bestDist = dist;
		bestIdx = i;
	    }
	}
	return std::make_pair(bestDist, bestIdx);
    }

    // checks +/- 100 around pos.
    std::pair<int32_t, size_t> checkNearPos(std::vector<uint32_t> &fp, std::string dbFilename, size_t pos, std::vector<uint32_t> &db, size_t posInDb) {

	if (posInDb > nearRange && posInDb < db.size()-nearRange-macro_sz) {
	    std::vector<uint32_t> window(db.begin()+posInDb-nearRange, db.begin()+posInDb+nearRange+macro_sz);
	    int32_t dist = 0;
	    size_t idx = 0;

	    std::tie(dist, idx) = fullCheck(fp, window);

	    return std::make_pair(dist, idx + posInDb-nearRange);
	}
	
	size_t begin = 0;
	size_t end = pos + nearRange + macro_sz;
	if (pos > nearRange) begin = pos - nearRange;

	std::ifstream fin(dbFilename.c_str(), std::ifstream::in | std::ifstream::binary);
	fin.seekg(0, std::ios_base::end);
	size_t fileEnd = fin.tellg();
	if (end*4 > fileEnd) {
	    end = fileEnd/4;
	}
	fin.seekg(sizeof(uint32_t)*begin);

	std::unique_ptr<uint32_t[]> buf(new uint32_t[end-begin]);
	fin.read((char*) buf.get(), sizeof(uint32_t)*(end-begin));

	std::vector<uint32_t> window(end-begin, 0);
	for (size_t i = 0; i < end-begin; ++i) {
	    window[i] = buf[i];
	}
	int32_t dist = 0;
	size_t idx = 0;

	std::tie(dist, idx) = fullCheck(fp, window);

	return std::make_pair(dist, idx + begin);

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

        //for (size_t i = 0; i < end-macro_sz; ++i,++pos) {
	for (size_t i = 0; i < end-macro_sz; i+=8,pos+=8) {
            if (pos - prevMatchPos < (this->fp_strategy->getSampleRate()/64) &&
		prevMatchPos != std::numeric_limits<size_t>::max()) continue;

            size_t dist = 0;
	    size_t bestMatchPos = std::numeric_limits<size_t>::max();
	    size_t startJ = 50;
	    bool earlyTermination = false;
	    
	    std::tie(earlyTermination, dist) = hammingEarlyTerminate(fingerprints, db, i);

	    if (earlyTermination) continue;

	    std::tie(dist, bestMatchPos) = checkNearPos(fingerprints, this->dbFilename,
							pos, db, i);
	    
	    //dist = fullHamming(fingerprints, db, i);
	    //std::tie(earlyTermination, dist) = hammingEarlyTerminate(fingerprints, db, i);
	    if (!earlyTermination && dist < macro_sz*sizeof(uint32_t)*0.35*8) {
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
	    if (i + nearRange > end-macro_sz) {
		size_t tmp = end-macro_sz-i;
		i = i + tmp;
		pos = pos + tmp;
	    } else {
		i+=nearRange; pos+=nearRange;
	    }
        }
    }

}

void si::fingerprint_db::query_preprocessed(std::string filename, std::vector<std::string> &results) {

    //TODO: make this work.

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
