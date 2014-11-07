#ifndef FINGERPRINT_DB_GUARD
#define FINGERPRINT_DB_GUARD

#include <fingerprint_strategy.hh>
#include <map>
#include <string>
#include <vector>

namespace sound_index {
    class fingerprint_db {
    private:
        std::map<size_t, std::string> idToFile;
    
        std::string dbFilename;

        fingerprint_strategy *fp_strategy;

    public:

        fingerprint_db();

        void open(std::string filename);

        void close();

        void insert(std::string filename, std::string indexedName = "");

	void query_preprocessed(std::string filename, std::vector<std::string> &results);

        void query_scan(std::string filename, std::vector<std::string> &results);

        void query(std::string filename, std::vector<std::string> &results);
    
        void merge(std::vector<std::string> &inputs);

    };
}

#endif //FINGERPRINT_DB_GUARD
