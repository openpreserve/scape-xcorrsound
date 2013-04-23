#ifndef DB_WRAPPER
#define DB_WRAPPER

#include <utility>
#include <vector>
#include <string>

#include "FingerprintInfo.h"

class db_wrapper {

public:
    db_wrapper(char dbFile[]);
    ~db_wrapper();

    int insert(size_t fingerprint, FingerprintInfo &info);
    int bulk_insert(std::vector<std::pair<size_t, FingerprintInfo> > &data);
    int query(size_t fingerprint, std::vector<FingerprintInfo> &result);
    uint32_t insert_file(std::string filename);
    std::string getFilename(size_t id);
};

#endif
