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

	std::vector<std::string> query_scan(std::string filename);
    
	void merge(fingerprint_db &other);


    };
}

#endif //FINGERPRINT_DB_GUARD
