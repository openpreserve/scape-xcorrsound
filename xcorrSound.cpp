#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <fftw3.h>
#include <complex>

#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

#include <unistd.h>
#include <stdlib.h>
#include "stdint.h"

#include "my_utils.h"

#include "logstream.h"

static const double THRESHOLD = 0.2;
static const double EPS = 1e-6;

typedef long long ll;

typedef long double float_type;
typedef std::complex<float_type> complex_type;

using namespace std;

int hz, channels;

logstream ls(3, "xcorrSound.log");

struct wav_header {
    unsigned char ChunkID[4];
    uint32_t ChunkSize;
    unsigned char Format[4];
    
    void print() {
	ls << log_information();
	ls << "----------------------- wav header start -----------------------" << endl;
	ls << "ChunkID: " << ChunkID << endl;
	ls << "ChunkSize: " << ChunkSize << endl;
	ls << "Format: " << Format << endl;
	ls << "----------------------- wav header end -----------------------" << endl;
    }

};

struct wav_subchunk {
    unsigned char Subchunk1ID[4];
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    unsigned char Subchunk2ID[4];
    uint32_t Subchunk2Size;

    void print() {
	ls << log_information();
	ls << "----------------------- wav subchunk start -----------------------" << endl;
	ls << "Subchunk1ID: " << Subchunk1ID << endl;
	ls << "Subchunk1Size: " << Subchunk1Size << endl;
	ls << "AudioFormat: " << AudioFormat << endl;
	ls << "NumChannels: " << NumChannels << endl;
	ls << "SampleRate: " << SampleRate << endl;
	ls << "ByteRate: " << ByteRate << endl;
	ls << "BlockAlign: " << BlockAlign << endl;
	ls << "BitsPerSample: " << BitsPerSample << endl;
	ls << "Subchunk2ID: " << Subchunk2ID << endl;
	ls << "Subchunk2Size: " << Subchunk2Size << endl;
	ls << "----------------------- wav subchunk end -----------------------" << endl;
    }

};

void printUsage() {
    cout << "Usage:" << endl;
    cout << "./xcorrSound <file1> <file2> [v<log level>]" << endl;
    cout << "Log levels:" << endl;
    cout << "1: fatal" << endl;
    cout << "2: errors" << endl;
    cout << "3: information" << endl;
    cout << "4: warning" << endl;
    cout << "5: debug" << endl;
    cout << "Logging information will be appended to the file xcorr.log" << endl;
    exit(1);
}

void read_header(wav_header &header, ifstream &f) {
    f.seekg(0);
    unsigned char *buffer = new unsigned char[12];
    f.read((char*)buffer,12);
    for (int i = 0; i < 4; ++i) {
	header.ChunkID[i] = buffer[i];
    }
    header.ChunkSize = convertFourBytesToInt(buffer[4], buffer[5], buffer[6], buffer[7]);
    for (int i = 8; i < 12; ++i) {
	header.Format[i-8] = buffer[i];
    }
    delete[] buffer;
}

void read_subchunk(wav_subchunk &subchunk, ifstream &f) {
    f.seekg(12);
    char *buffer = new char[32];
    f.read((char*)buffer,32);

    for (int i = 0; i < 4; ++i) {
	subchunk.Subchunk1ID[i] = buffer[i];
    }

    subchunk.Subchunk1Size = convertFourBytesToInt(buffer[4],buffer[5],buffer[6],buffer[7]);
    subchunk.AudioFormat = convertTwoBytesToShort(buffer[8],buffer[9]);
    subchunk.NumChannels = convertTwoBytesToShort(buffer[10],buffer[11]);
    subchunk.SampleRate = convertFourBytesToInt(buffer[12],buffer[13],buffer[14],buffer[15]);
    subchunk.ByteRate = convertFourBytesToInt(buffer[16],buffer[17],buffer[18],buffer[19]);
    subchunk.BlockAlign = convertTwoBytesToShort(buffer[20],buffer[21]);
    subchunk.BitsPerSample = convertTwoBytesToShort(buffer[22],buffer[23]);

    for (int i = 24; i < 28; ++i) {
	subchunk.Subchunk2ID[i-24] = buffer[i];
    }
    
    subchunk.Subchunk2Size = convertFourBytesToInt(buffer[28],buffer[29],buffer[30],buffer[31]);

}

void readAudioFile(char *buffer, short* arr, ifstream &f, int size, int start) {
    f.seekg(start);
    f.read(buffer,size); 
    convertCharArrayToShort(buffer, arr, size);
}

void prefixSum(short * a, ll *spa, size_t size) {
    spa[0] = a[0]*a[0];
    spa[1] = a[1]*a[1];
    for (size_t i = 2; i < size; i += 2) {
	spa[i] = spa[i-2]+((ll)a[i]*(ll)a[i]);
	spa[i+1] = spa[i-1] + ((ll)a[i+1]*(ll)a[i+1]);
    }
}

/**
 * ...
 */
double computeNormFactor(ll *spa1, ll *spa2, int j, size_t a1Size, size_t a2Size) {
    // still using left samples.
    ll a1End = spa1[a1Size-2];
    ll a1Start = 0;
    if (a1Size-j-4 >= 0) {
	a1Start = spa1[a1Size-j-4];
    }

    //double a1s = ((double) (a1End - a1Start));
    ll a1s = a1End - a1Start;
    ll a2s = spa2[a2Size-2];
    if (j < a2Size) {
	a2s = spa2[j];
    }

    //double a2s = ((double) spa2[j]);
    
    //double ans = sqrt(0.5*a1s+0.5*a2s);
    //double ans = 0.5*a1s + 0.5*a2s;
    double ans = 0.5*a1s + 0.5*a2s;
    return ans;
}

void xcorr_new(short *a1, short *a2, size_t a1Size, size_t a2Size, vector<complex_type> &out) {
    fftw_complex *in1,*in2, *in3, *out1, *out2, *out3;
    fftw_plan p1,p2,p3;

    in1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a2Size/2);
    in2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a1Size/2);
    in3 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * min(a1Size/2,a2Size/2));
    out1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a2Size/2);
    out2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * a1Size/2);
    out3 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * min(a1Size/2,a2Size/2));

    for (size_t i = 0; i < a1Size/2; ++i) {
	in2[i][0] = a1[i*2];
	in2[i][1] = 0;

    }

    for (size_t i = 0; i < a2Size/2; ++i) {
	in1[i][0] = a2[i*2];
	in1[i][1] = 0;

    }

    p1 = fftw_plan_dft_1d(a2Size/2, in1, out1, FFTW_FORWARD, FFTW_ESTIMATE);
    p2 = fftw_plan_dft_1d(a1Size/2, in2, out2, FFTW_FORWARD, FFTW_ESTIMATE);
    p3 = fftw_plan_dft_1d(min(a1Size,a2Size)/2, in3, out3, FFTW_FORWARD, FFTW_ESTIMATE);
    
    ls << log_debug() << "starting first fft" << endl;

    fftw_execute(p1);

    ls << "first fft done" << endl;

    fftw_free(in1);

    ls << "starting second fft" << endl;

    fftw_execute(p2);

    ls << "second fft done" << endl;

    fftw_free(in2);

    for (int i = 0; i < min(a1Size,a2Size)/2; ++i) {
	in3[i][0] = out1[i][0] * out2[i][0] + out1[i][1] * out2[i][1];
	in3[i][1] = -out1[i][0] * out2[i][1] + out1[i][1] * out2[i][0];
    }
    fftw_free(out1);
    fftw_free(out2);

    ls << "starting third fft" << endl;

    fftw_execute(p3);

    ls << "third fft done" << endl;

    fftw_destroy_plan(p1);
    fftw_destroy_plan(p2);
    fftw_destroy_plan(p3);

    out.resize(min(a1Size,a2Size)/2);
    for (size_t i = 0; i < out.size(); ++i) {
	out[i] = complex_type(out3[i][0]/out.size(),out3[i][1]);
    }
    fftw_free(out3);
}

void doWork(short *a1, short *a2, size_t a1Size, size_t a2Size, size_t a1PaddingSize, size_t toAdd, bool outputGraph = false) {

    vector<complex_type > cross(min(a1Size,a2Size)/2, complex_type(0.0,0.0));

    xcorr_new(a1,a2,a1Size,a2Size,cross);
    
    ls << log_debug() << "prefix sum 1 computing..." << flush << endl;

    ll *spa1 = new ll[a1Size];
    prefixSum(a1,spa1,a1Size);

    ls << "prefix sum 1 done" << endl;
    ls << "prefix sum 2 computing..." << endl;

    ll *spa2 = new ll[a2Size];
    prefixSum(a2,spa2,a2Size);

    ls << "prefix sum 2 done" << endl;

    delete[] a1;
    delete[] a2;

    double max = 0;
    size_t maxSampleNumber = 0;
    double maxSec = 0.0;
    for (size_t sample_number = a1PaddingSize; sample_number < min(a1Size,a2Size)/2; ++sample_number) {
	double sec = ((double)sample_number-a1PaddingSize+toAdd)/hz;
	double normFactor = computeNormFactor(spa1, spa2, 2*sample_number, a1Size, a2Size);

	if (outputGraph) {
	    cout << sec << '\t' << cross[sample_number].real() << '\t' << normFactor+1 << endl;//'\t';
	} else {
	    double ans = cross[sample_number].real()/normFactor;
	    if (abs(ans) > max && abs(ans) <= 1+EPS) { // last part is sanity check
		max = abs(ans);
		maxSampleNumber = sample_number - a1PaddingSize;
		maxSec = sec;
	    }
	}

    }


    if (!outputGraph) {
	if (max > THRESHOLD) {
	    cout << "Match found." << endl;
	    cout << "Match at sample number: " << maxSampleNumber << endl;
	    cout << "Match at second: " << maxSec << endl;
	    cout << "Value of match was: " << max << endl;
	} else {
	    cout << "The best match was below the threshold of " << THRESHOLD << endl;
	    cout << "Best at sample number: " << maxSampleNumber << endl;
	    cout << "Best at second: " << maxSec << endl;
	    cout << "Value of match was: " << max << endl;
	}
    }

    delete[] spa1;
    delete[] spa2;

}

void doFromFile(int argc, char *argv[]) {
    if (argc < 3) {
	printUsage();
	return;
    }

    time_t tmp = time(NULL);

    ls << log_information() << "run started at: " << ctime(&tmp) << endl;

    string file1(argv[1]), file2(argv[2]);

    if (argc >= 4 && argv[3][0] == 'v' && argv[3][1] != '\0') {
	int level = argv[3][1]-'0';
	if (level >= 1 && level <= 5) {
	    ls.set_print_level(level);
	}
    }
    
    ifstream f1,f2;
    f1.open(file1.c_str(), ifstream::binary);
    f2.open(file2.c_str(), ifstream::binary);

    wav_header *header1 = new wav_header, *header2 = new wav_header;
    wav_subchunk *subchunk1 = new wav_subchunk, *subchunk2 = new wav_subchunk;
    read_header(*header1,f1);
    read_subchunk(*subchunk1,f1);

    read_header(*header2,f2);
    read_subchunk(*subchunk2,f2);

    header1->print();
    subchunk1->print();

    header2->print();
    subchunk2->print();

    if (subchunk1->SampleRate != subchunk2->SampleRate) {
	ls << log_fatal() << "Sample rates differ, exiting." << endl;
	exit(1);
    }

    hz = subchunk1->SampleRate;

    int a1Size = getFilesize(file1)-44;
    int a2Size = getFilesize(file2)-44;

    int sz = min(a1Size,a2Size);

    char *buffer = new char[sz];
    short *a1 = new short[sz];

    ls << log_debug() << "reading file 1" << endl;

    readAudioFile(buffer, a1+sz/2, f1, sz, a1Size-sz+44);
    delete[] buffer;

    buffer = new char[sz];
    short *a2 = new short[sz];

    ls << log_debug() << "reading file 2" << endl;

    readAudioFile(buffer, a2, f2, sz, 44);
    delete[] buffer;

    doWork(a1,a2,sz,sz, sz/4, a1Size/4-sz/4);

    tmp = time(NULL);
    ls << log_information() << "run finished at: " << ctime(&tmp) << endl;

}

int main(int argc, char *argv[]) {

    // read two files from stdin and output to file their 
    // normalized cross correlation as a function of lag.

    doFromFile(argc, argv);
    
    return 0;
}
