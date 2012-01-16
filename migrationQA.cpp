#include <vector>
#include <complex>
#include "stdint.h"
#include <iostream>
#include "logstream.h"

#include "cross_correlation.h"
#include "AudioFile.h"

#include "boost/program_options.hpp"

using namespace std;

namespace po = boost::program_options;

logstream ls(5, "migrationQA.log");

void printUsage() {
    cout << "Usage: ./migrationQA <first.wav> <second.wav>" << endl << endl;
}

void printVersion() {

}

void printInfo() {
    cout << "=== Info ===" << endl << endl;
    cout << "This program produces output that can be used to do quality assurance." << endl;
    cout << "The program outputs an off-set value and a match value for each 'chunk'" << endl;
    cout << "where the two audio files by default are split into 5 seconds chunks." << endl << endl;

    cout << "Output:" << endl;
    cout << "block i: <match value> <off-set>" << endl << endl;

    cout << "One way to use these numbers is to verify that all chunks have the same off-set" << endl;
    cout << "and that all chunks have a good match value (close to 1)" << endl << endl;
}

int main(int argc, char *argv[]) {

    string input1, input2;

    po::options_description generic("Generic options");
    generic.add_options()
	("help,h", "Print help message")
	("version,v", "Print version")
	("info", "Print info")
	;

    po::options_description hidden("Hidden options");
    hidden.add_options()
	("input-files", po::value<vector<string> >(), "Input files, there must be exactly 2")
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
	return 0;
    }
    if (vm.count("version")) {
	//printVersion();
	return 0;
    }
    if (vm.count("info")) {
	printInfo();
	return 0;
    }
    if (vm.count("input-files")) {
	vector<string> f = vm["input-files"].as<vector<string> >();
	if (f.size() != 2) {
	    std::cout << generic << std::endl;
	    return 1;
	}
	input1 = f[0];
	input2 = f[1];
    } else {
	std::cout << generic << std::endl;
	return 1;
    }

    AudioFile a(input1.c_str());
    AudioFile b(input2.c_str());

    //cross correlate 5 seconds at a time.
    
    uint32_t samplesPr5Seconds = a.getSampleRate() * 5;
    ls << log_debug() << "samples a: " << samplesPr5Seconds << endl;
    ls << log_debug() << "samples b: " << b.getSampleRate() * 5 << endl;
    AudioStream aStream = a.getStream(0);
    AudioStream bStream = b.getStream(0);

    bool done = false;
    bool first = true;
    double firstMaxVal = 0.0;
    size_t firstMaxIdx = -1;
    for (int block = 1; !done; ++block) {
	vector<short> aSamples, bSamples;
	vector<uint64_t> aSquarePrefixSum, bSquarePrefixSum;
	vector<complex<double> > result;

	aStream.read(samplesPr5Seconds, aSamples);
	bStream.read(samplesPr5Seconds, bSamples);

	prefixSquareSum(aSamples, aSquarePrefixSum);
	prefixSquareSum(bSamples, bSquarePrefixSum);

	proxyFFT<short, double> aFFT(aSamples);
	proxyFFT<short, double> bFFT(bSamples);

	cross_correlation(aFFT, bFFT, result);

	double maxVal = 0.0;
	size_t maxIdx = -1;
	for (size_t i = 0; i < result.size(); ++i) {
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

	if (first) {
	    first = false;
	    firstMaxVal = maxVal;
	    firstMaxIdx = maxIdx;
	} else {
	    if (maxIdx - firstMaxIdx > 100 || firstMaxIdx - maxIdx > 100) {
		//done = true;
	    }
	}

	if (aSamples.size() < samplesPr5Seconds || bSamples.size() < samplesPr5Seconds) {
	    done = true;
	}
	cout << "block " << block << ": " << maxVal << " " << maxIdx << endl;

    }
}
