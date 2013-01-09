#ifndef MY_QUERY_H
#define MY_QUERY_H

#include <string>
#include <vector>
#include <complex>
#include <stdint.h>
#include "my_database.h"
#include "../my_utils.h"
#include "../cross_correlation.h"

class my_query {
private:
    typedef size_t index_t;
    
    std::string queryFile;
    my_database &db;
    double find_peak(std::vector<uint64_t> &prefixA, 
		     std::vector<uint64_t> &prefixB,
		     std::vector<std::complex<double> > &output);
	
    std::vector<int16_t> getSamplesFromIndex(index_t index);
    
    double cross_correlate(std::vector<int16_t>::iterator aBegin, std::vector<int16_t>::iterator aEnd,
			   std::vector<int16_t>::iterator bBegin, std::vector<int16_t>::iterator bEnd);
    
    std::vector<size_t> retrieveIndices(const my_fingerprint &queryFingerprint);
public:
    my_query(std::string queryFile, my_database &db);
    std::vector<std::pair<size_t, double> > execute();
};

#endif
