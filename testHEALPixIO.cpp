#include <cassert>
#include <cmath>
#include <iostream>

#include <healpix_map.h>
#include <pointing.h>
#include <healpix_map_fitsio.h>
#include <datatypes.h>

int main()
{
    // create a test map
    typedef Healpix_Map<double> mapType;

    mapType m0;
    mapType m1;
    mapType m2;

    int nSide = 1024;

    m0.SetNside(nSide,RING);
    m1.SetNside(nSide,RING);
    m2.SetNside(nSide,RING);

    m0.fill(double(1));
    m1.fill(double(2));
    m2.fill(double(3));

    // write map
    write_Healpix_map_to_fits("!./test_healpix_io.fits",m0,m1,m2,planckType<double>());

    // read this back now
    mapType t0;
    mapType t1;
    mapType t2;

    read_Healpix_map_from_fits("./test_healpix_io.fits",t0,int(1),int(2));
    read_Healpix_map_from_fits("./test_healpix_io.fits",t1,int(2),int(2));
    read_Healpix_map_from_fits("./test_healpix_io.fits",t2,int(3),int(2));

    for(auto pix =0;pix<m0.Npix();++pix)
    {
        //assert( std::abs(m0[pix]-t0[pix]) < 1e-12 );
        if(std::abs(m0[pix]-t0[pix]) > 1e-12)
        {
            std::cout<<"Pixel values of m0 mismatch"<<"\t"<<pix<<"\t"<<m0[pix]<<"\t"<<t0[pix]<<std::endl;
            return -1;
        }

        if(std::abs(m1[pix]-t1[pix]) > 1e-12)
        {
            std::cout<<"Pixel values of m1 mismatch"<<"\t"<<pix<<"\t"<<m1[pix]<<"\t"<<t1[pix]<<std::endl;
            return -1;
        }

        if(std::abs(m2[pix]-t2[pix]) > 1e-12)
        {
            std::cout<<"Pixel values of m2 mismatch"<<"\t"<<pix<<"\t"<<m2[pix]<<"\t"<<t2[pix]<<std::endl;
            return -1;
        }
    }

    return 0;
}
