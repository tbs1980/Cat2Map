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

/**
 * \class Cat2Map
 * \brief A class for converting catalogues to HEALPix maps
 *
 * This class reads a catalogue and convert the information to a
 * HEALPix map.
 */
class Cat2Map
{
public:

    /**
     * \typedef boost::property_tree::ptree propertyTreeType
     * \brief defines the property tree type
     */
    typedef boost::property_tree::ptree propertyTreeType;

    /**
     * \typedef Healpix_Map<double> mapType
     * \brief defines the HEALPix map type
     */
    typedef Healpix_Map<double> mapType;

    static const constexpr double deg2rad = M_PI/double(180);/**< factor for converting degree to radians */
    static const constexpr double rotPhi = double(0); /**< rotaion to phi required for convertion from ra */

    /**
     * \brief The default constructor
     *
     * \param iniFileName name of the input configuration file
     */
    explicit Cat2Map(std::string const& iniFileName)
    {
        BOOST_LOG_TRIVIAL(info) << std::string("Reading the ini file ") + std::string(iniFileName);
        boost::property_tree::ini_parser::read_ini(iniFileName,mPropTree);

        // set the output maps
        BOOST_LOG_TRIVIAL(info) << "Resulution of the output map is "<<mPropTree.get<std::string>("output.n_side");

        int nSide = mPropTree.get<int>("output.n_side");
        mMapN.SetNside(nSide,RING);
        mMapNNInv.SetNside(nSide,RING);
        mMapE1.SetNside(nSide,RING);
        mMapE2.SetNside(nSide,RING);
        mMapE1NInv.SetNside(nSide,RING);
        mMapE2NInv.SetNside(nSide,RING);

        mMapN.fill(double(0));
        mMapNNInv.fill(double(0));
        mMapE1.fill(double(0));
        mMapE1.fill(double(0));
        mMapE1NInv.fill(double(0));
        mMapE2NInv.fill(double(0));

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

    /**
     * \brief A function that accumulates the objects in a catalogue
     */
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
                                mMapE1NInv[pix] += e1*e1;
                                mMapE2NInv[pix] += e2*e2;
                                //mNumObsPix += size_t(1); //TODO this is wrong
                            }
                        }

                    }

                }
                else
                {
                    BOOST_LOG_TRIVIAL(info) << "Skipping line " << line_id;
                }
            }

            // make it the average , also count the total objects
            double totObjs(0);
            for(auto pix = 0; pix<mMapN.Npix(); ++pix)
            {
                if(mMapN[pix]>0) // TODO how many gals we need to make an estimate
                {
                    mMapE1[pix] /= mMapN[pix];
                    mMapE2[pix] /= mMapN[pix];

                    // var = E(X^2) - E(X)^2
                    mMapE1NInv[pix] = ( mMapE1NInv[pix]/mMapN[pix] - mMapE1[pix]*mMapE1[pix]);
                    mMapE2NInv[pix] = ( mMapE2NInv[pix]/mMapN[pix] - mMapE2[pix]*mMapE2[pix]);

                    // nInv = 1/var
                    if(mMapE1NInv[pix] > double(0) and mMapE2NInv[pix] > double(0) )
                    {
                        mMapE1NInv[pix] = double(1)/mMapE1NInv[pix];
                        mMapE2NInv[pix] = double(1)/mMapE2NInv[pix];

                        // compute the observed number of pixels in the data
                        mNumObsPix += size_t(1);
                        totObjs += mMapN[pix];

                        //mMapNNInv[pix] = nbar; //TODO note that I may have to recalculate this again
                    }
                    else
                    {
                        // we don't have the variance properly defined
                        mMask[pix] = 0;
                        mMapE1NInv[pix] = 0.;
                        mMapE2NInv[pix] = 0;
                        mMapE1[pix] = 0;
                        mMapE2[pix] = 0;
                        mMapN[pix] = 0;
                        mMapNNInv[pix] = 0;
                    }
                }
                else
                {
                    // make the new mask
                    mMask[pix] = 0;
                    mMapE1NInv[pix] = 0.;
                    mMapE2NInv[pix] = 0;
                    mMapE1[pix] = 0;
                    mMapE2[pix] = 0;
                    mMapNNInv[pix] = 0;
                }
            }

            // output the sky fraction of the augmented mask
            size_t nPix = (size_t) mTestE1.Npix();
            double fKsy = (double)mNumObsPix / (double)nPix;
            BOOST_LOG_TRIVIAL(info) << "Sky fraction " << fKsy;

            BOOST_LOG_TRIVIAL(info) << "Toal objects in the map " << totObjs;

            // output the n_bar or the mean number of objects per pixel
            double nBar = totObjs/(double)mNumObsPix;
            BOOST_LOG_TRIVIAL(info) << "Mean objects per pixel " << nBar;

            assert(nBar > double(0));

            // assign nbar to nInv map for number density
            for(auto pix = 0; pix<mMapN.Npix(); ++pix)
            {
                if(mMapN[pix] > 0)
                {
                    mMapN[pix] = (mMapN[pix] - nBar)/nBar;
                    mMapNNInv[pix] = nBar;
                }
            }

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

    /**
     * \brief write the maps to fits files
     */
    void writeMaps()
    {
        BOOST_LOG_TRIVIAL(info) << "Output data map file name :  "
            << mPropTree.get<std::string>("output.data_map_file_name");

        write_Healpix_map_to_fits(std::string("!")+mPropTree.get<std::string>("output.data_map_file_name"),
            mMapN,mMapE1,mMapE2,planckType<double>());

        BOOST_LOG_TRIVIAL(info) << "Output nInv map file name :  "
            << mPropTree.get<std::string>("output.nInv_map_file_name");

        write_Healpix_map_to_fits(std::string("!")+mPropTree.get<std::string>("output.nInv_map_file_name"),
            mMapNNInv,mMapE1NInv,mMapE2NInv,planckType<double>());

        BOOST_LOG_TRIVIAL(info) << "Output augmented mask file name :  "
            << mPropTree.get<std::string>("output.augmented_mask_file_name");

        write_Healpix_map_to_fits(std::string("!")+mPropTree.get<std::string>("output.augmented_mask_file_name"),
            mMask,planckType<double>());
    }

private:
    propertyTreeType mPropTree; /**< property tree that stores the ini file information */
    mapType mMapN; /**< number density map */
    mapType mMapNNInv; /**< number density map */
    mapType mMapE1; /**<  ellipticity-1 map */
    mapType mMapE2; /**< ellipticity-2 map */
    mapType mMapE1NInv; /**<  ellipticity-1 nInv map */
    mapType mMapE2NInv; /**< ellipticity-2 nInv map */
    mapType mMask; /**< mask */
    std::vector<double> mZBounds; /**< z bounds */

    mapType mTestE1; /**<  ellipticity-1 test map*/
    mapType mTestE2; /**< ellipticity-2 test map */
    bool mDoTest; /**< a flag to to sanity test */
    size_t mMisMatchCountE1; /**< sanity check mismatch for ellipticity-1*/
    size_t mMisMatchCountE2; /**< sanity check mismatch for ellipticity-2*/
    size_t mNumObsPix; /**< number of observed pixels*/

};


#endif //CAT2MAP_HPP
