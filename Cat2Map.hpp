#ifndef CAT2MAP_HPP
#define CAT2MAP_HPP

#include <fstream>
#include <iostream>
#include <exception>
#include <vector>
#include <cmath>
#include <cassert>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/log/trivial.hpp>
#include <boost/tokenizer.hpp>

#include <healpix_map.h>
#include <pointing.h>
#include <healpix_map_fitsio.h>
#include <datatypes.h>

class Cat2Map
{
public:
    typedef boost::property_tree::ptree propertyTreeType;
    typedef Healpix_Map<double> mapType;

    static const constexpr double deg2rad = M_PI/double(180);

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

            int nSide = mPropTree.get<int>("output.n_side");
            mMapN.SetNside(nSide,RING);
            mMapE1.SetNside(nSide,RING);
            mMapE2.SetNside(nSide,RING);

            mMapN.fill(double(0));
            mMapE1.fill(double(0));
            mMapE1.fill(double(0));

            //std::cout<<"num pix = "<<mMapN.Npix()<<std::endl;

            BOOST_LOG_TRIVIAL(info) << "Accumulating objects";
            BOOST_LOG_TRIVIAL(info) << "Number of rows to be skipped = " << mPropTree.get<std::string>("input.skip_rows");
            BOOST_LOG_TRIVIAL(info) << "Delimiter for separation is "<< mPropTree.get<std::string>("input.delimiter");

            size_t col_ra =  mPropTree.get<size_t>("input.col_ra");
            size_t col_dec =  mPropTree.get<size_t>("input.col_dec");
            size_t col_z = mPropTree.get<size_t>("input.col_z");
            size_t col_ellip_1 = mPropTree.get<size_t>("input.col_ellip_1");
            size_t col_ellip_2 = mPropTree.get<size_t>("input.col_ellip_2");

            BOOST_LOG_TRIVIAL(info) << "Column of ra values = "<< col_ra;
            BOOST_LOG_TRIVIAL(info) << "Column of dec values = "<< col_dec;
            BOOST_LOG_TRIVIAL(info) << "Column of z values = "<< col_z;
            BOOST_LOG_TRIVIAL(info) << "Column of ellip_1 values = "<< col_ellip_1;
            BOOST_LOG_TRIVIAL(info) << "Column of ellip_2 values = "<< col_ellip_2;


            size_t line_id = 0;
            while(!inputCatFile.eof())
            {
                std::string line;

                std::getline(inputCatFile,line);

                ++line_id;

                if(line_id > mPropTree.get<size_t>("input.skip_rows"))
                {
                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(mPropTree.get<std::string>("input.delimiter").c_str());
                    tokenizer tokens(line, sep);

                    std::vector<double> ents;
                    for(tokenizer::iterator tokIter = tokens.begin(); tokIter !=tokens.end(); ++tokIter)
                    {
                        ents.push_back(boost::lexical_cast<double>(*tokIter));
                    }

                    //assert(ents.size() >0);
                    //assert(ents.size()<=8);

                    if(ents.size()>0)
                    {
                        double ra = ents[col_ra];
                        double dec = ents[col_dec];
                        double e1 = ents[col_ellip_1];
                        double e2 = ents[col_ellip_2];

                        double theta = -deg2rad*dec + M_PI*double(0.5);
                        double phi = deg2rad*(ra - double(180.));

                        auto pix = mMapE1.ang2pix(pointing(theta,phi));

                        mMapN[pix] += double(1);
                        mMapE1[pix] += e1;
                        mMapE2[pix] += e2;
                    }

                }
                else
                {
                    BOOST_LOG_TRIVIAL(info) << "Skipping line " << line_id;
                }
            }

            // make it the average
            for(auto pix = 0; pix<mMapN.Npix(); ++pix)
            {
                if(mMapN[pix]>1)
                {
                    mMapE1[pix] /= mMapN[pix];
                    mMapE2[pix] /= mMapN[pix];
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
        BOOST_LOG_TRIVIAL(info) << "Output file is  " << mPropTree.get<std::string>("output.output_file_name");
        write_Healpix_map_to_fits(std::string("!")+mPropTree.get<std::string>("output.output_file_name"),mMapN,mMapE1,mMapE2,planckType<double>());
    }

private:
    propertyTreeType mPropTree;
    mapType mMapN;
    mapType mMapE1;
    mapType mMapE2;

};


#endif //CAT2MAP_HPP
