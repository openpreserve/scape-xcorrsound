#include <boost/program_options.hpp>
#include <cstdlib>
#include <fingerprint_db.hh>
#include <fstream>
#include <iostream>
#include <string>

namespace po = boost::program_options;
namespace si = sound_index;

namespace {

    std::string dbname;
    std::string file;
    std::string input;

    si::fingerprint_db db;

}

void bulk() {

    db.open(dbname);

    std::ifstream fin(file.c_str(), std::ifstream::in);

    if (!fin) {
        std::cout << "Could not open input file specified with -f." << std::endl;
        exit(1);
    }

    std::string x;

    while (fin >> x) {
        // progress info
        std::cout << x << std::endl;
        db.insert(x);
    }

    db.close();

}

void single() {

    db.open(dbname);
    db.insert(input);
    db.close();

}

void init(int argc, char *argv[]) {
    po::options_description generic("Program options");
    generic.add_options()
	("help,h", "Print help message and return, everything else is ignored");
    
    po::options_description hidden("Settings");
    hidden.add_options()
	("dbname,d", po::value<std::string>(), "Database name")
	("file,f", po::value<std::string>(), "File with names of wav files for bulk insertion, this ignores the --input option")
	("input,i", po::value<std::string>(), "Single wav file");

    po::options_description all("Allowed options");
    all.add(generic).add(hidden);

    po::options_description visible("Program options");
    visible.add(generic);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).run(), vm);

    if (vm.count("help")) {
        std::cout << "Usage: ismir_build_index [PROGRAM OPTIONS] SETTINGS" << std::endl << std::endl;
        std::cout << generic << std::endl;

        std::cout << hidden << std::endl;

        exit(0);
    }

    if (vm.count("dbname")) {
        dbname = vm["dbname"].as<std::string>();
    } else {
        std::cout << "dbname must be provided" << std::endl;
        exit(1);
    }

    if (vm.count("file")) {
        file = vm["file"].as<std::string>();
        bulk();
        exit(0);
    }
    if (vm.count("input")) {
        input = vm["input"].as<std::string>();
        single();
        exit(0);
    }

    std::cout << "need at least one input file" << std::endl;
    exit(1);


}

int main(int argc, char *argv[]) {
    init(argc, argv);
    return 0;
}
