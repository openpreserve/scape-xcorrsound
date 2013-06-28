#include <boost/program_options.hpp>
#include <cstdlib>
#include <fingerprint_db.hh>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;
namespace si = sound_index;

void init(int argc, char *argv[]) {
    std::vector<std::string> dbFiles;
    std::string outputfile;

    po::options_description generic("Program options");
    generic.add_options()
        ("help,h", "Print help message and return. Everything else is ignored");

    po::positional_options_description positional;
    positional.add("input-databases", -1);

    po::options_description hidden("Settings");
    hidden.add_options()
        ("output-database,o", po::value<std::string>(), "Output database name, should not already exist")
        ("input-databases,i", po::value<std::vector<std::string> >(), "List of input databases to merge");

    po::options_description all("all");
    all.add(generic).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(positional).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: ismir_merge [SETTINGS]" << std::endl;
        std::cout << generic << std::endl;
        std::cout << hidden << std::endl;
        std::cout << "Both input and output database names must be specified" << std::endl << std::endl;
        return;
    }

    if (vm.count("output-database") && vm.count("input-databases")) {
        
        std::string outputdbName = vm["output-database"].as<std::string>();
        std::vector<std::string> inputdbs = vm["input-databases"].as<std::vector<std::string> >();

        for (size_t i = 0; i < inputdbs.size(); ++i) {
            std::cout << inputdbs[i] << std::endl;
        }

        si::fingerprint_db outputdb;
        outputdb.open(outputdbName);
        outputdb.merge(inputdbs);
        outputdb.close();
        return;
    } else {

        std::cout << "Both input and output database names must be specified" << std::endl;
        exit(1);

    }

}

int main(int argc, char *argv[]) {
    init(argc, argv);
    return 0;
}
