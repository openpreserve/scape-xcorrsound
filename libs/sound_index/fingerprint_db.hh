#ifndef FINGERPRINT_DB_GUARD
#define FINGERPRINT_DB_GUARD

#include <map>
#include <string>
#include <vector>

namespace sound_index {
    class fingerprint_db {
    private:
        std::map<size_t, std::string> idToFile;
    
        std::string dbFilename;

    public:

        void open(std::string filename);

        void close();

        void insert(std::string filename);

        void query_scan(std::string filename, std::vector<std::string> &);
    
        void merge(std::vector<std::string> &inputs);

    };
}

#endif //FINGERPRINT_DB_GUARD
