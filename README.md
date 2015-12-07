# Cat2Map
A C++ code for convgerting a galaxy catalogue to a HELPixMap

## Denpendencies

1. [Boost](http://www.boost.org/)
2. [HEALPix](http://healpix.sourceforge.net/)
3. [CFITSIO](http://heasarc.gsfc.nasa.gov/fitsio/)
4. [CMake](https://cmake.org/)

## Build instructions

We require CMake to build the executables. The paths to Boost,HEALPix and CFITSIO can be passed at the build time as shown below.

	$git clone https://github.com/tbs1980/Cat2Map.git
	$cd Cat2Map
	$mkdir build
	$cd build
	$cmake ../ -DCMAKE_PREFIX_PATH="path_to_boost;path_to_healpix;path_to_cfitsio"
	
## Example

The arguments to `Cat2Map` is an `ini` file with correct specifications. The maps can be made by

	$Cat2Map example.ini

In general the `ini` file will look like

```ini
[input]
; full path to input catalogue file
catlogue_file_name = ./test-ellip-noise0-catalog_100k.dat
; full path to the input mask
mask_file_name = ./euclid_mask.fits
; number of rows to be skipped while reading the catalogue file
skip_rows = 1
; delimiter of the catalogue file
delimiter = " "
; column corresponding to ra
col_ra = 0
; column corresponding to dec
col_dec = 1
; column corresponding to z
col_z = 2
; column corresponding to ellipticity 1
col_ellip_1 = 6
; column corresponding to ellipticity 2
col_ellip_2 = 7

[output]
; HEALPix resolution of the output map
n_side = 1024
; full path to the output file for writing the data map
data_map_file_name = ./test-ellip-noise0-catalog_100k.fits
; full path to the output file for writing the augmented mask
augmented_mask_file_name = ./euclid_aug_mask.fits
```

An example catalogue file format is shown below

	#  ra dec z kappa gamma1 gamma2 ellip1 ellip2
		40.020103     -79.708305     0.99461716   -0.023294616   -0.014624927   0.0083968099   -0.014292001   0.0082056625 
		40.109955     -79.707863      1.0331188   -0.027732531   -0.016495496  0.00099063676   -0.016050378  0.00096390524 
		40.346882      -79.70549     0.98424661   -0.028683368   -0.019208925  -0.0071485126   -0.018673312  -0.0069491868 
		40.526699     -79.708893      1.0228475   -0.018730894   -0.020199986   -0.009220086   -0.019828578  -0.0090505611 
		44.319969       -79.7099     0.99026835   0.0079140151   -0.028427923   0.0035343564   -0.028654696   0.0035625505 
		44.417561     -79.707756      1.0198203   0.0004159268   -0.023948593   0.0065980251   -0.023958558   0.0066007706 
		44.629353     -79.713165      1.0236109  -0.0081229536   -0.011430762    0.014662232   -0.011338659    0.014544091 
		44.771263     -79.711884      1.0239419   -0.010177474  -0.0094703306    0.016185584  -0.0093749175    0.016022515 
		44.716331     -79.707947      1.0297984   -0.010177474  -0.0094703306    0.016185584  -0.0093749175    0.016022515 
		44.825916     -79.711952     0.96221447   -0.014020724  -0.0095437663    0.014031175  -0.0094118062    0.013837167 
		44.871864     -79.701172      1.0095795   -0.014020724  -0.0095437663    0.014031175  -0.0094118062    0.013837167 
		44.941822     -79.715691     0.98328984   -0.019197885  -0.0091443472   0.0085198171  -0.0089721018   0.0083593354 
		45.073326     -79.709381     0.97038674   -0.022463646  -0.0083685946   0.0012608439  -0.0081847357    0.001233143 
