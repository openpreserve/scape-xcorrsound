#include <complex>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "stdint.h"
#include <sstream>
#include <vector>

#include "boost/program_options.hpp"

#include "AudioFile.h"
#include "cross_correlation.h"
#include "logstream.h"
#include "my_utils.h"

using namespace std;

namespace po = boost::program_options;

logstream ls(5, "waveform-compare.log");

// input parameters
string input1, input2;
bool verbose;
uint32_t secondsPrBlock = 1;
uint32_t channel = 0;
double threshold = 0.97;

void printUsage() {
    cout << "Usage: waveform-compare <first.wav> <second.wav> [--block-size=N] [--threshold=X]" << endl;
    cout << "Where N is the (integer) number of seconds pr block and X is the threshold in" << endl;
    cout << "[0,1]. By default N=5 and X=0.98. Optionally the channel to compare can be" << endl;
    cout << "specified as --channel=k where k is the channel number (i.e. 0 would be the left channel)." << endl;
    cout << "By default k=0" << endl << endl;
}

void printVersion() {
    cout << "waveform-compare version 0.1.0" << endl;
}

void printInfo() {
    cout << "=== Info ===" << endl << endl;
    cout << "This program produces output that can be used to do quality assurance." << endl;
    cout << "If the verbose flag is set the program outputs an off-set value and a" << endl;
    cout << "match value for each 'chunk' where the two audio files by default are" << endl;
    cout << "split into 5 seconds chunks." << endl << endl;

    cout << "Output:" << endl;
    cout << "block i: <match value> <off-set>" << endl << endl;

    cout << "Explanation of 'offset':" << endl;
    cout << "Denote the first input file as A[0...n-1]," << endl;
    cout << "and the second input file as B[0...n-1]." << endl;
    cout << "Let the output be positive and denote it k" << endl;
    cout << "This means the best match is when we overlay the two files such that" << endl;
    cout << "A[k] = B[0], A[k+1] = B[1] ..." << endl;
    cout << "If the offset was negative (-k) the best match would be when" << endl;
    cout << "A[0] = B[k], A[1] = B[k+1] ..." << endl << endl;

    cout << "Pictures to describe the two situations. First k >= 0, second k < 0:" << endl << endl;
    cout << "Picture 1:" << endl;
    cout << " 0 1 ... k ... n-1" << endl;
    cout << "+-----------------+" << endl;
    cout << "|                 |          A" << endl;
    cout << "+-----------------+" << endl << endl;
    cout << "        +-----------------+ " << endl;
    cout << "        |                 |  B" << endl;
    cout << "        +-----------------+" << endl;
    cout << "         0 1 .. .. ... n-1" << endl << endl;
    cout << "Picture 2:" << endl;
    cout << "         0 1 .. .. ... n-1" << endl;
    cout << "        +-----------------+ " << endl;
    cout << "        |                 |  A" << endl;
    cout << "        +-----------------+ " << endl;
    cout << " 0 1 ... k ... n-1" << endl;
    cout << "+-----------------+" << endl;
    cout << "|                 |          B" << endl;
    cout << "+-----------------+" << endl << endl;

    cout << "One way to use these numbers is to verify that all chunks have the same off-set" << endl;
    cout << "and that all chunks have a good match value (close to 1)" << endl << endl;


    cout << "The program outputs either 'success' or 'failure' followed by an explanation" << endl;
    cout << "Success is when all blocks matched within 500 samples of the first block's offset" << endl;
    cout << "On failure there will be a message saying during which timestamp the offset" << endl;
    cout << "did not line up." << endl << endl;
}

void init(int argc, char *argv[]) {

    po::options_description generic("Program options");
    generic.add_options()
	("help,h", "Print help message")
	("version,v", "Print version")
	("info", "Print info")
	("verbose", "Verbose output")
	;

    po::options_description hidden("Hidden options");
    hidden.add_options()
	("input-files", po::value<vector<string> >(), "Input files, there must be exactly 2")
	("block-size", po::value<int32_t>(), "Block size in seconds (must be integer) when comparing, default is 5")
	("threshold", po::value<double>(), "Threshold value, default 0.98")
	("channel", po::value<int32_t>(), "Channel, default 0")
	;
    
    po::positional_options_description positional;
    positional.add("input-files", -1);

    po::options_description all("Allowed options");
    all.add(generic).add(hidden);

    po::options_description visible("Allowed options"); //showed to the user
    visible.add(generic);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(positional).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
	std::cout << generic << std::endl;
	printUsage();
	exit(0);
    }
    if (vm.count("version")) {
	printVersion();
	exit(0);
    }
    if (vm.count("info")) {
	printInfo();
	exit(0);
    }

    if (vm.count("verbose")) {
	verbose = true;
    } else {
	verbose = false;
    }

    if (vm.count("block-size")) {
	int32_t block_size = vm["block-size"].as<int32_t>();
	secondsPrBlock = block_size;
    }

    if (vm.count("threshold")) {
	threshold = vm["threshold"].as<double>();
    }

    if (vm.count("channel")) {
	channel = vm["channel"].as<int32_t>();
    }

    if (vm.count("input-files")) {
	vector<string> f = vm["input-files"].as<vector<string> >();
	if (f.size() != 2) {
	    std::cout << generic << std::endl;
	    exit(1);
	}
	input1 = f[0];
	input2 = f[1];
    } else {
	std::cout << generic << std::endl;
	exit(1);
    }

}

pair<int64_t, double> 
compareBlock(proxyFFT<int16_t, double> &aFFT, proxyFFT<int16_t, double> &bFFT,
	     vector<uint64_t> &aSquarePrefixSum, vector<uint64_t> &bSquarePrefixSum) {

    vector<complex<double> > result;
    pair<int64_t, double> res = make_pair(-1,0.0);

    cross_correlation(aFFT, bFFT, result);

    double maxVal = 0.0;
    int64_t maxIdx = -1;

    // results are unreliable if there is not enough 'overlap', hence -1000.
    for (size_t i = 0; i < result.size()-1000; ++i) {
	double val = result[i].real();
	double norm = computeNormFactor(aSquarePrefixSum, bSquarePrefixSum, 
					aSquarePrefixSum.begin() + i, aSquarePrefixSum.end(),
					bSquarePrefixSum.begin(), bSquarePrefixSum.end() - i);

	val /= norm;
	if (val > maxVal + 1e-6) {
	    maxVal = val;
	    maxIdx = i;
	}
    }

    res.first = maxIdx;
    res.second = maxVal;
    return res;

}

int main(int argc, char *argv[]) {

    init(argc, argv);

    AudioFile a(input1.c_str());
    AudioFile b(input2.c_str());

    if (channel >= a.getNumberOfChannels() || channel >= b.getNumberOfChannels()) {
	ls << log_error() << "Error, channel not available" << endl;
	return 1;
    }

    //cross correlate x seconds at a time.

    uint32_t samplesPrBlock = a.getSampleRate() * secondsPrBlock;
    ls << log_debug() << "samples a: " << samplesPrBlock << endl;
    ls << log_debug() << "samples b: " << b.getSampleRate() * secondsPrBlock << endl;
    AudioStream aStream = a.getStream(channel);
    AudioStream bStream = b.getStream(channel);

    bool done = false;
    bool first = true;
    bool success = true;
    size_t blockFailure = 0;

    double firstMaxVal = 0.0;
    int64_t firstOffset = 0;

    for (size_t block = 1; !done; ++block) {
	vector<int16_t> aSamples, bSamples;
	vector<uint64_t> aSquarePrefixSum, bSquarePrefixSum;
	vector<complex<double> > result;

	aStream.read(samplesPrBlock, aSamples);
	bStream.read(samplesPrBlock, bSamples);

	if (aSamples.size() < samplesPrBlock/2 || bSamples.size() < samplesPrBlock) {
	    // not enough samples for another reliable check.
	    break;
	}

	prefixSquareSum(aSamples, aSquarePrefixSum);
	prefixSquareSum(bSamples, bSquarePrefixSum);

	// we count the average of absolute values 
	// if the average is close to 0, then we decide it is silence

	size_t absSumA = 0; size_t absSumB = 0;
	bool silence = false;
	for (size_t i = 0; i < aSamples.size(); ++i) {
	    absSumA += (aSamples[i]>=0)?aSamples[i]:-aSamples[i];
	}
	for (size_t i = 0; i < bSamples.size(); ++i) {
	    absSumB += (bSamples[i]>=0)?bSamples[i]:-bSamples[i];
	}
	
	double avgA = static_cast<double>(absSumA)/aSamples.size();
	double avgB = static_cast<double>(absSumB)/bSamples.size();

	if (avgA <= 5.0 && avgB <= 5.0) {
	    silence = true;
	} else if (avgA <= 5.0 || avgB <= 5.0) {
	    success = false;
	} 

	bool compare = !silence;

	int64_t maxIdx = -1; double maxVal = -1.0;

	if (compare) {
	    proxyFFT<int16_t, double> aFFT(aSamples);
	    proxyFFT<int16_t, double> bFFT(bSamples);

	    pair<int64_t, double> tmp = compareBlock(aFFT, bFFT, aSquarePrefixSum, bSquarePrefixSum);

	    maxIdx = tmp.first;
	    maxVal = tmp.second;

	    tmp = compareBlock(bFFT, aFFT, bSquarePrefixSum, aSquarePrefixSum);
	
	    if (tmp.second > maxVal + 1e-6) {
		maxIdx = -tmp.first;
		maxVal = tmp.second;
	    }

	    if (first) {
		first = false;
		firstMaxVal = maxVal;
		firstOffset = maxIdx;
	    } else {
		if (maxIdx - firstOffset > 500 || firstOffset - maxIdx > 500 || maxVal < threshold) {
		    // check to see that the offset between blocks is not too large.
		    success = false;
		    blockFailure = block;
		}
	    }
	}

	if (aSamples.size() < samplesPrBlock || bSamples.size() < samplesPrBlock) { 
	    // we don't check the last block. Is this ok?
	    done = true;
	}
	if (verbose) {
	    if (compare) {
		cout << "block " << block << ": " << maxVal << " " << maxIdx << endl;
	    } else if (silence) {
		cout << "block " << block << ": " << "silence" << endl;
	    }
	}
    }

    if (success) {
	cout << "Success" << endl;
	cout << "Offset: " << firstOffset << endl;
	return 0;
    } else {
	cout << "Failure" << endl;
	cout << "Block: " << blockFailure << endl;
	return 1;
	/*
	cout << "block " << blockFailure << ":" << endl;
	cout << "Time: " << getTimestampFromSeconds(blockFailure*5-5) << " - " 
	     << getTimestampFromSeconds(blockFailure*5) << " did not match properly" << endl;
	*/
    }

}
