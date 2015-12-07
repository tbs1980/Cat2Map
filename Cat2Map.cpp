#define BOOST_ALL_DYN_LINK

#include <boost/program_options.hpp>

#include "Cat2Map.hpp"

namespace po = boost::program_options;

int main(int ac, char* av[])
{
    try
    {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("input-file,f", po::value< std::string >(), "input file");

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 0;
        }
        else if (vm.count("input-file"))
        {
            Cat2Map c2m(vm["input-file"].as< std::string >());
            c2m.accumulate();
            c2m.writeMaps();
        }
        else
        {
            std::cout<<":( No arguments passed. Please type Cat2Map --help"<<std::endl;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
