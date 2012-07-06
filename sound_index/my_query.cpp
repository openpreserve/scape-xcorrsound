#include <vector>
#include <algorithm>
#include <iostream>
#include <complex>
#include <cassert>
#include "stdint.h"

#include "my_query.h"
#include "../AudioFile.h"
#include "../AudioStream.h"
#include "my_database.h"
#include "../cross_correlation.h"
#include "../my_utils.h"

using namespace std;

double
my_query::find_peak(vector<int32_t> &prefixA, vector<int32_t> &prefixB, vector<complex<double> > &output) {
    double maxVal = 0.0;
    size_t maxIdx = 0;
    bool flag = false;
    for (size_t i = 0; i < output.size(); ++i) {
	double val = output[i].real();
	double norm = computeNormFactor(prefixA, prefixB, 
					prefixA.begin() + i, prefixA.end(),
					prefixB.begin(), prefixB.end() - i);
	val /= norm;
	if (val > maxVal) {
	    maxVal = val;
	    maxIdx = i;
	    flag = true;
	}
    }

    if (flag) {
	assert(maxVal <= 1.0);
	return maxVal;
    }    
}

double
my_query::cross_correlate(vector<int16_t>::iterator aBegin, vector<int16_t>::iterator aEnd,
			  vector<int16_t>::iterator bBegin, vector<int16_t>::iterator bEnd) {

    vector<int32_t> prefixA, prefixB;
    prefixSquareSum<int16_t, int32_t>(aBegin, aEnd, prefixA);
    prefixSquareSum<int16_t, int32_t>(bBegin, bEnd, prefixB);
    
    proxyFFT<int16_t, double> aProxy(aBegin, aEnd);
    proxyFFT<int16_t, double> bProxy(bBegin, bEnd);

    vector<complex<double> > output;

    cross_correlation(aProxy, bProxy, output);
    double max_val = find_peak(prefixA, prefixB, output);
    output.clear();
    cross_correlation(bProxy, aProxy, output);
    max_val = max(max_val, find_peak(prefixB, prefixA, output));

    return max_val;

}

vector<size_t>
my_query::retrieveIndices(const my_fingerprint &queryFingerprint) {
    return db.retrieveIndices(queryFingerprint);
}

my_query::my_query(std::string queryFile, my_database &db) : queryFile(queryFile), db(db) {
}

vector<size_t>
my_query::execute() {
    AudioFile qaf(queryFile.c_str());
    AudioStream qas = qaf.getStream(0);
    vector<int16_t> samples;
    qas.read(qaf.getNumberOfSamplesPrChannel(), samples);

    // take 3 fingerprints 1 second apart.
    
    uint32_t oneSecond = qaf.getSampleRate();

    if (samples.size() < 4 * oneSecond) {
	// error handling. Assume the query is always 4 seconds or more
    }

    my_fingerprint f1(samples.begin()+oneSecond, samples.begin() + oneSecond + 2048);
    // my_fingerprint f2(samples.begin()+oneSecond*2, samples.begin() + oneSecond*2 + 2048);
    // my_fingerprint f3(samples.begin()+oneSecond*3, samples.begin() + oneSecond*3 + 2048);

    vector<size_t> f1Results = retrieveIndices(f1);
    // vector<size_t> f2Results = retrieveIndices(f2);
    // vector<size_t> f3Results = retrieveIndices(f3);

    // std::sort(f1Results.begin(), f1Results.end());

    // vector<pair<size_t, int> > intersection(f1.size());
    // for (size_t i = 0; i < f1Results.size(); ++i) {
    // 	intersection[i].first = f1Results[i];
    // 	intersection[i].second = 1;
    // }

    // for (size_t i = 0; i < f2Results.size(); ++i) {
    // 	pair<size_t, int> key = make_pair(f2Results[i], 0);
    // 	vector<pair<size_t, int> >::iterator it = lower_bound(intersection.begin(), intersection.end(), key);
    // 	if (it->first == f2Results[i]) {
    // 	    it->second = it->second + 1;
    // 	}
    // }
    // vector<size_t> goodCandidates;
    // for (size_t i = 0; i < intersection.size(); ++i) {
    // 	if (intersection[i].second == 3) {
    // 	    goodCandidates.push_back(intersection[i].first);
    // 	}
    // }

    vector<size_t> matches;
    for (size_t i = 0; i < f1Results.size(); ++i) {

	vector<int16_t> possibleMatchSamples = getSamplesFromIndex(f1Results[i]);

	if (possibleMatchSamples.size() != 2048) {
	    // error handling.
	}

	double val = cross_correlate(possibleMatchSamples.begin(), possibleMatchSamples.end(),
				     samples.begin()+oneSecond, samples.begin() + oneSecond + 2048);

	if (val > 0.7) {
	    matches.push_back(f1Results[i]);
	}
    }
    return matches;
}

vector<int16_t>
my_query::getSamplesFromIndex(index_t index) {
    return db.getSamplesFromIndex(index);
}
