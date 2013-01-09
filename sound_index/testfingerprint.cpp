#include <iostream>
#include <vector>
#include <complex>
#include <map>
#include <sstream>
#include <cstdlib>
#include <sys/time.h>
#include <stdint.h>

#include "fingerprint.h"
#include "../AudioFile.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) return 1;
    AudioFile afDB(argv[1]);

    vector<short> samples;
    afDB.getSamplesForChannel(0, samples);
    std::cout << "done reading" << std::endl;
    map<uint32_t, vector<uint32_t> > hashes;
    //const size_t SKIP = 16384;
    const size_t SKIP = 18368;//65536;//32768;
    size_t prevWrite = 0;

    for (size_t i = 0; i < samples.size()/SKIP; ++i) {

	vector<complex<double> > transformed;
	vector<short>::iterator begin, end;
	begin = samples.begin()+i*SKIP;
	end = samples.begin()+(i+1)*SKIP;
	if (end > samples.end()) { end = samples.end(); }
	vector<short> input(begin, end);

	Fingerprint FP(input);

	if (!(i % 50)) {
	    std::cout << "i: " << i << std::endl;
	} 

	for (size_t j = 0; j < 256; ++j) {
	    vector<uint32_t> F(FP.getFingerprint());
	    if (hashes.count(F[j])) {
		hashes[F[j]].push_back(j);
	    } else {
		hashes.insert(make_pair(F[j], vector<uint32_t>(1,i)));
	    }
	}

	//hashes[ss.str()] = i*SKIP;
	if (i > 0) {
	    // progress information.
	    size_t total = samples.size()/SKIP;
	    size_t curr = i*100 / total;
	    if (curr - prevWrite*100/total >= 5) {
		//cout << curr << "%" << endl;
		//cout << "furthermore: sizeof(hashes) = " << sizeof(hashes) << ", .size() = " << hashes.size() << endl;
		prevWrite = i;
		
	    }
	    //if (
	} else {
	    //cout << "0%" << endl;
	}

    }

    typedef map<uint32_t, vector<uint32_t> >::iterator myit;
    double avg = 0.0;
    size_t count = 0;
    for (myit iter = hashes.begin(); iter != hashes.end(); ++iter) {
	cout << (*iter).first;
	for (size_t i = 0; i < (*iter).second.size(); ++i) {
	    cout << " " << (*iter).second[i];
	}
	cout << endl;
	avg += (*iter).second.size();
	++count;
    }
    avg = avg / count;
    cout << avg << endl;
/*
    AudioFile afQuery(argv[2]);
    samples.resize(0);
    afQuery.getSamplesForChannel(0, samples);
    for (int i = 3; i < argc; ++i) {
	cout << "Query " << i-2 << endl;
	size_t q = atol(argv[i]);
	vector<complex<double> > transformed;
	vector<short>::iterator begin, end;
	begin = samples.begin() + q;
	end = begin+2048;
	bool match = false;
	for (size_t k = 0; k < SKIP; ++k) {
	    begin = samples.begin() + q + k;
	    end = begin+2048;
	    vector<short> input(begin, end);
	    computeFFT(input, transformed);
	    hash96bit hash(transformed.begin(), transformed.end());
	    stringstream ss;
	    for (size_t j = 0; j < 32; ++j) {
		int box = hash.hashString->get(3*j) + 2*hash.hashString->get(3*j+1) + 4*hash.hashString->get(3*j+2);
		ss << box;
	    }

	    if (hashes.count(ss.str())) {
		//cout << '\t' << "match at sample: " << hashes[ss.str()] << " sec ~= " << hashes[ss.str()]/44100 << endl;
		//cout << "\tk+q = " << k << "+" << q << " = " << k+q << endl;
		cout << "\t" << "Match at hash: " << ss.str() << endl;
		cout << "\t" << "Number of hits: " << hashes[ss.str()].size() << endl;
		if (hashes[ss.str()].size() == 1) {
		    cout << "\t\t" << "Match was at ~" << hashes[ss.str()][0]/44100.0 << "s" << endl;
		}
		match = true;
	    }
	}
	if (!match) {
	    cout << "\t" << "No matches found." << endl;
	}
    }
*/
}
