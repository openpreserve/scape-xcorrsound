#include <AudioFile.h>
#include <AudioStream.h>
#include <complex>
#include <computeFFT.h>
#include <cstring>
#include <fftw3.h>
#include <fingerprint_db.hh>
#include <fingerprint_stream.hh>
#include <fstream>
#include <hamming.h>
#include <iomanip>
#include <map>
#include <my_utils.h>
#include <ostream>
#include <set>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

using std::vector; using std::complex; using std::map; 
using std::set; using std::string; using std::stringstream;
using std::cin; using std::cout; using std::endl;
using std::pair; using std::ofstream; using std::ifstream;

using namespace sound_index;

vector<uint32_t> db;
map<size_t, string> files;
int fileNumber = 0;
size_t cnt = 0;

static const double PI = 3.14159265359;

namespace {

    void writeDBToDisk(std::string dbFilename) {

	std::ofstream of(dbFilename.c_str(), std::ios::out | std::ios::binary | std::ios::app);
	size_t init = of.tellp() / 4;
	for (size_t i = 0; i < db.size(); ++i) {
	    of.write((char*) &db[i], sizeof(uint32_t));
	}
	size_t end = of.tellp() / 4;
	of.close();

	std::string mapFilename = "map_" + dbFilename;
	std::ofstream mof(mapFilename.c_str(), std::ios::out | std::ios::app);

	typedef std::map<size_t, string>::iterator m_iter;
	for (m_iter iter = files.begin(); iter != files.end(); ++iter) {
	    mof << iter->first + init << " " << iter->second << endl;
	}

	mof.close();

    }

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

}

/* debug function */
inline
void printbinary(uint32_t n) {
    for (size_t i = 32; i > 0; --i) {
        cout << ((n>>(i-1)) & 1);
    }
    cout << endl;
}

uint32_t NumberOfSetBits(uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

uint32_t hammingLookup16(uint32_t n) {
    uint16_t a = n & 0xFFFF;
    uint16_t b = n>>16;

    return hamming16[a] + hamming16[b];
}

inline
static uint32_t hammingWeight(uint32_t n) {

    n = ((n>>1) & 0x55555555) + (n & 0x55555555);

    n = ((n>>2) & 0x33333333) + (n & 0x33333333);

    n = ((n>>4) & 0x0F0F0F0F) + (n & 0x0F0F0F0F);

    n = ((n>>8) & 0x000F000F) + (n & 0x000F000F);

    n = ((n>>16) & 0x000000FF) + (n & 0x000000FF);

    return n;
}
/*
void generateFingerprintStream2(vector<int16_t> &input, vector<uint32_t> &output) {
    size_t begin = 0, end = frameLength;

    vector<double> hanningWindow;
    getHanningWindow(frameLength, hanningWindow);

    while (end < input.size()) {
        
        vector<double> frame(input.begin() + begin, input.begin() + end);
        
        double rms = 0.0;
        for (size_t i = 0; i < frame.size(); ++i) {
            rms += frame[i]*frame[i];
        }
        rms /= frameLength;
        rms = sqrt(rms);
        
        for (size_t i = 0; i < frame.size(); ++i) {
            frame[i] = (frame[i]/rms) * hanningWindow[i];
        }

        vector<complex<double> > transform;
        computeFFT(frame, transform);

        double maxFreq = 0.0;
        size_t maxFreqIdx = 0;
        size_t secondIdx = 0;
        // hz = idx * sampleRate/frameLength
        // idx = hz * frameLength / sampleRate
        size_t val = ceil(318.0*frameLength/5512.0);
        for (size_t i = 0; i < transform.size()/2; ++i) {
            if (abs(transform[i]) > maxFreq) {
                secondIdx = maxFreqIdx;
                maxFreqIdx = i;
                maxFreq = abs(transform[i]);
            }
            //if (i < 10) cout << setw(8) << abs(transform[i]);
        }
        //cout << endl;

        uint32_t fp = (maxFreqIdx << 10) | secondIdx;
        //uint32_t fp = secondIdx;
        output.push_back(fp);
        begin += advance; end += advance;
    }

}
*/

void query(const char *file) {

    AudioFile a(file);
    
    vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);

    size_t bestMatch = db.size();
    size_t bestMatchDistance = fingerprints.size();

    for (size_t i = 0; i < db.size() - fingerprints.size(); ++i) {

        //uint32_t distance = 0;
	uint32_t dist2 = 0;
        //for (size_t j = 0; j < fingerprints.size(); ++j) {
	for (size_t j = fingerprints.size()/2; j < fingerprints.size()/2+32; ++j) {
            uint32_t x = static_cast<uint32_t>(fingerprints[j] ^ db[i+j]);
            //uint32_t cnt = hammingWeight(x);
	    //uint32_t cnt = NumberOfSetBits(x);
	    uint32_t cnt = hammingLookup16(x);
	    dist2 += cnt;
            //if (cnt > 10) ++distance;

        }
        if (dist2 < bestMatchDistance) {
            bestMatchDistance = dist2;
            bestMatch = i;
        }
        //if (distance < fingerprints.size()*3/4) {
	//if (distance <= 17) {
	if (dist2 <= 358) { //0.35 * 1024 ~= 358
            map<size_t, string>::iterator iter = files.lower_bound(i);
	    
	    //size_t fileId = iter->first;
	    
	    string filename = iter->second;

	    size_t start = 0;
	    if (iter != files.begin()) {
		--iter;
		start = iter->first;
	    }

	    size_t sampleInFile = i - start;

	    string timestamp = getTimestampFromSeconds(sampleInFile*advance / sampleRate);
	    cout << "match in " << filename 
		 << " at " << timestamp
		 << " with distance " << dist2 << endl;
	    
	    //cout << i << '\t' << 10 << endl;

	}
    }

    map<size_t, string>::iterator iter = files.lower_bound(bestMatch);

    string filename = iter->second;

    size_t start = 0;
    if (iter != files.begin()) {
        --iter;
        start = iter->first;
    }
    
    size_t sampleInFile = bestMatch - start;
    
    string timestamp = getTimestampFromSeconds(sampleInFile*advance / sampleRate);

    cout << "best match in " << filename << " at " << timestamp << " with distance " << bestMatchDistance << endl;
    cout << sampleInFile << endl;

}

void insert_file(const char *file) {

    AudioFile a(file);
    
    vector<int16_t> samples;

    a.getSamplesForChannel(0, samples);

    vector<uint32_t> fingerprints;

    generateFingerprintStream(samples, fingerprints);

    db.insert(db.end(), fingerprints.begin(), fingerprints.end());

    files.insert(make_pair(db.size(), string(file)));
    
}


int main(int argc, char *argv[]) {

    if (argc < 2) {
	cout << "usage:" << endl;
	cout << "./ismir_impl <# db files> <# query files> [db file 1,.., db file n] [q file 1,.. q file m]" << endl << endl;
	cout << "If #db files > 0, they are appended to file string.bin and map_string.bin" << endl << endl;
        return 1;
    }

   
    size_t numDBfiles = static_cast<size_t>(atoi(argv[1]));
    size_t numQueryfiles = static_cast<size_t>(atoi(argv[2]));

    for (size_t i = 3; i < numDBfiles+3; ++i) {
        insert_file(argv[i]);
        cout << "db: " << db.size() << endl;
    }

    if (numDBfiles > 0) {
        writeDBToDisk("string_ismir.bin");
    } else {
        readDBFromDisk("string_ismir.bin", db);
    }

    cout << "db size: " << db.size() << endl;

    for (size_t i = numDBfiles+3; i < numDBfiles+numQueryfiles+3; ++i) {
        query(argv[i]);
    }

}
