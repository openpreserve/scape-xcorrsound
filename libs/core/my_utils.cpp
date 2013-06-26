#include "my_utils.h"

#include <string>
#include <sstream>

#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

using namespace std;

size_t timeDiff(timeval &tvStart, timeval &tvEnd) {
    size_t elapsedMS = (tvEnd.tv_usec + tvEnd.tv_sec*1000000) - (tvStart.tv_usec + tvStart.tv_sec*1000000);
    return elapsedMS;
}

int16_t getIntFromChars(uint8_t a, uint8_t b) {
    int16_t res = b;
    res = res << 8;
    res += a;
    return res;
}

int32_t getIntFromChars(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    int32_t res = 0;
    res = res | d;
    res = res << 8;
    res = res | c;
    res = res << 8;
    res = res | b;
    res = res << 8;
    res = res | a;
    return res;
}

int getFilesize(string filename) {
    struct stat filestatus;
    stat(filename.c_str(), &filestatus);
    return filestatus.st_size;
}

short convertTwoBytesToShort(char a, char b) {
    //little endian. a is the least significant byte.
    short res = b & 255;
    res = res << 8;
    res = res | (a&255);
    return res;
}

void printint(int n) {
    for (int i = 31; i >= 0; --i) {
	if ((n>>i) & 1) {
	    printf("%d",1);
	} else {
	    printf("%d",0);
	}
    }
    printf("\n");
}

void printchar(char n) {
    for (int i = 7; i >= 0; --i) {
	if ((n>>i) & 1) {
	    printf("%d",1);
	} else {
	    printf("%d",0);
	}
    }
    printf("\n");    
}

uint32_t convertFourBytesToInt(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
    uint32_t res = 0;
    int di = (0 | d) & 255;
    int ci = (0 | c) & 255;
    int bi = (0 | b) & 255;
    int ai = (0 | a) & 255;

    res = res | di;
    res = res << 8;
    res = res | ci;
    res = res << 8;
    res = res | bi;
    res = res << 8;
    res = res | ai;
    
    return res;
}

void convertCharArrayToShort(char* arr, short* arr2, size_t arraySize) {
    for (size_t i = 0; i < arraySize; i += 4) {
	arr2[i/2] = convertTwoBytesToShort(arr[i],arr[i+1]);
	arr2[i/2+1] = convertTwoBytesToShort(arr[i+2],arr[i+3]);
    }
}

string getTimestampFromSeconds(size_t seconds) {
    size_t hours = seconds/3600;
    size_t minutes = (seconds - 3600*hours)/60;
    seconds = seconds - 3600*hours - 60*minutes;

    stringstream tstmp;
    if (hours < 10) tstmp << '0';
    tstmp << hours << ":";
    if (minutes < 10) tstmp << '0';
    tstmp << minutes << ":";
    if (seconds < 10) tstmp << '0';
    tstmp << seconds;

    return tstmp.str();
}

void getHanningWindow(size_t windowLength, vector<double> &window) {

    const double PI = 3.14159265359;

    window.resize(windowLength);
    
    for (size_t i = 0; i < windowLength; ++i) {
        window[i] = (25.0/46.0) - (21.0/46.0) * cos((2*PI*i)/(windowLength-1));
    }
}
