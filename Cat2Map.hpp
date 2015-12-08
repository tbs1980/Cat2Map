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
    static const constexpr double rotPhi = double(0);

    Cat2Map(std::string const& iniFileName)
    {
        BOOST_LOG_TRIVIAL(info) << std::string("Reading the ini file ") + std::string(iniFileName);
        boost::property_tree::ini_parser::read_ini(iniFileName,mPropTree);

        // set the output maps
        BOOST_LOG_TRIVIAL(info) << "Resulution of the output map is "<<mPropTree.get<std::string>("output.n_side");

        int nSide = mPropTree.get<int>("output.n_side");
        mMapN.SetNside(nSide,RING);
        mMapE1.SetNside(nSide,RING);
        mMapE2.SetNside(nSide,RING);

        mMapN.fill(double(0));
        mMapE1.fill(double(0));
        mMapE1.fill(double(0));

        // read the mask
        read_Healpix_map_from_fits(mPropTree.get<std::string>("input.mask_file_name"),mMask);

        try
        {
            std::string testMapFileName = mPropTree.get<std::string>("test.map_file_name");

            BOOST_LOG_TRIVIAL(info) << "Test map file specified as "<< testMapFileName;

            read_Healpix_map_from_fits(testMapFileName,mTestE1,int(2),int(2));
            read_Healpix_map_from_fits(testMapFileName,mTestE2,int(3),int(2));

            mDoTest = true;
        }
        catch(std::exception)
        {
            BOOST_LOG_TRIVIAL(info) << "No test map file specified.";
            mDoTest = false;
        }


        try
        {
            std::string testMapFileName = mPropTree.get<std::string>("output.z_bounds");
            BOOST_LOG_TRIVIAL(info) << "z bounds specified as "<< testMapFileName;
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep(",");
            tokenizer tokens(testMapFileName, sep);


            for(tokenizer::iterator tokIter = tokens.begin(); tokIter !=tokens.end(); ++tokIter)
            {
                mZBounds.push_back(boost::lexical_cast<double>(*tokIter));
            }
        }
        catch(std::exception)
        {
            BOOST_LOG_TRIVIAL(info) << "No z bounds specified. All objects will be accumulated.";
        }

        if(mZBounds.size() != size_t(2))
        {
            std::string msg("The z-bounds should consist of two values. No more, no less.");
            throw std::runtime_error(msg);
        }

        if(mZBounds[0] >= mZBounds[1])
        {
            std::string msg("The upper bound should be greater than the lower bound.");
            throw std::runtime_error(msg);
        }

        mMisMatchCountE1 = size_t(0);
        mMisMatchCountE2 = size_t(0);
        mNumObsPix = size_t(0);
    }

    void accumulate()
    {
        // open file for reading
        std::ifstream inputCatFile;
        inputCatFile.open( mPropTree.get<std::string>("input.catlogue_file_name").c_str(),std::ios::in );


        if(inputCatFile.is_open())
        {

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

                    if(ents.size()>0)
                    {
                        assert(col_ra<ents.size());
                        assert(col_dec<ents.size());
                        assert(col_ellip_1<ents.size());
                        assert(col_ellip_2<ents.size());

                        double ra = ents[col_ra];
                        double dec = ents[col_dec];
                        double z = ents[col_z];
                        double e1 = ents[col_ellip_1];
                        double e2 = ents[col_ellip_2];

                        double theta = -deg2rad*dec + M_PI*double(0.5);
                        double phi = deg2rad*(ra - rotPhi);

                        auto pix = mMapE1.ang2pix(pointing(theta,phi));


                        // check if the pixel falls in the masked region
                        if(mMask[pix]>0)
                        {
                            if(z >= mZBounds[0] and z < mZBounds[1])
                            {
                                if(mDoTest)
                                {
                                    // if testing accumulate the miss-match
                                    // between pixel values
                                    if(std::abs( (e1 - mTestE1[pix])/e1 ) >= 1e-5)
                                    {
                                        mMisMatchCountE1 += size_t(1);
                                    }

                                    if(std::abs( (e2 - mTestE2[pix])/e2 ) > 1e-5)
                                    {
                                        mMisMatchCountE2 += size_t(1);
                                    }
                                }

                                // do accumulation
                                mMapN[pix] += double(1);
                                mMapE1[pix] += e1;
                                mMapE2[pix] += e2;
                                mNumObsPix += size_t(1);
                            }
                        }

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
                if(mMapN[pix]>0) // TODO how many gals we need to make an estimate
                {
                    mMapE1[pix] /= mMapN[pix];
                    mMapE2[pix] /= mMapN[pix];
                }
                else
                {
                    // make the new mask
                    mMask[pix] = 0;
                }
            }

            // output the sky fraction of the augmented mask
            size_t nPix = (size_t) mTestE1.Npix();
            double fKsy = (double)mNumObsPix / (double)nPix;
            BOOST_LOG_TRIVIAL(info) << "Sky fraction " << fKsy;

            // if testing print the mismatch stats
            if(mDoTest)
            {

                double fracMissMatchE1 = (double)mMisMatchCountE1 / (double)mNumObsPix;
                double fracMissMatchE2 = (double)mMisMatchCountE2 / (double)mNumObsPix;

                BOOST_LOG_TRIVIAL(warning) << "Pixel value miss-match for e1 = "<<fracMissMatchE1;
                BOOST_LOG_TRIVIAL(warning) << "Pixel value miss-match for e2 = "<<fracMissMatchE2;
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
        BOOST_LOG_TRIVIAL(info) << "Output data map file name :  "
            << mPropTree.get<std::string>("output.data_map_file_name");

        write_Healpix_map_to_fits(std::string("!")+mPropTree.get<std::string>("output.data_map_file_name"),
            mMapN,mMapE1,mMapE2,planckType<double>());

        BOOST_LOG_TRIVIAL(info) << "Output augmented mask file name :  "
            << mPropTree.get<std::string>("output.augmented_mask_file_name");

        write_Healpix_map_to_fits(std::string("!")+mPropTree.get<std::string>("output.augmented_mask_file_name"),
            mMask,planckType<double>());
    }

private:
    propertyTreeType mPropTree;
    mapType mMapN;
    mapType mMapE1;
    mapType mMapE2;
    mapType mMask;
    std::vector<double> mZBounds;

    mapType mTestE1;
    mapType mTestE2;
    bool mDoTest;
    size_t mMisMatchCountE1;
    size_t mMisMatchCountE2;
    size_t mNumObsPix;

};


#endif //CAT2MAP_HPP
