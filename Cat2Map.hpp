#ifndef CAT2MAP_HPP
#define CAT2MAP_HPP

#include <fstream>
#include <iostream>
#include <exception>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/log/trivial.hpp>

#include <healpix_map.h>

class Cat2Map
{
public:
    typedef boost::property_tree::ptree propertyTreeType;
    typedef Healpix_Map<double> mapType;

    Cat2Map(std::string const& iniFileName)
    {
        BOOST_LOG_TRIVIAL(info) << std::string("Reading the ini file ") + std::string(iniFileName);
        boost::property_tree::ini_parser::read_ini(iniFileName,mPropTree);
    }

    void accumulate()
    {
        // open file for reading
        std::ifstream inputCatFile;
        inputCatFile.open( mPropTree.get<std::string>("input.catlogue_file_name").c_str(),std::ios::in );

         
        if(inputCatFile.is_open())
        {
            BOOST_LOG_TRIVIAL(info) << "Resulution of the output map is "<<mPropTree.get<std::string>("output.n_side");
            BOOST_LOG_TRIVIAL(info) << "Accumulating objects";
            BOOST_LOG_TRIVIAL(info) << "Number of rows to be skipped = " << mPropTree.get<std::string>("input.skip_rows");
            BOOST_LOG_TRIVIAL(info) << "Delimiter for separation is "<< mPropTree.get<std::string>("input.delimiter");

            size_t line_id = 0;
            while(!inputCatFile.eof())
            {
                std::string line;

                std::getline(inputCatFile,line);

                ++line_id;
                
                if(line_id > mPropTree.get<size_t>("input.skip_rows"))
                {

                }
                else
                {
                    BOOST_LOG_TRIVIAL(info) << "Skipping line " << line_id;
                }
            }
        }
        else
        {
            std::string msg = std::string("Input catalogue file ")
                + std::string(mPropTree.get<std::string>("input.catlogue_file_name"))
                + std::string(" failed to open.");
            BOOST_LOG_TRIVIAL(error) << msg;
            throw std::runtime_error(msg);
        }
        
    }

    void writeMaps()
    {

    }

private:
    propertyTreeType mPropTree;
    mapType mMapN;
    mapType mMapE1;
    mapType mMapE2;

};


#endif //CAT2MAP_HPP