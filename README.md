# eic_endcap_insert

[![DOI](https://zenodo.org/badge/491633889.svg)](https://zenodo.org/badge/latestdoi/491633889)

Running the standalone simulation
---------------------------------
Enter the EIC container and source the default enviroment. Then do the following:

If you don't have them already, get the latest ip6 files and install them:
```
git clone https://eicweb.phy.anl.gov/EIC/detectors/ip6.git
cd ip6
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX -DCMAKE_CXX_STANDARD=17
make install
cd ../..
```
Then clone this repository and install it:
```
git clone https://github.com/rymilton/eic_endcap_insert.git
cd eic_endcap_insert
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX -DCMAKE_CXX_STANDARD=17
make install

cd ..
./run_sim_hepmc
```
This will run both the detector simulation step and the reconstruction (JUGGLER) step.
You can either use ./run_sim_hepmc or ./run_sim_gps, depending on if you want a hepmc generator or the general particle source.

## GPS documentation
---------------------------------
If you want to adjust the particle gun, here's the documentation for the general particle source (GPS):

   Manual: https://www.fe.infn.it/u/paterno/Geant4_tutorial/slides_further/GPS/GPS_manual.pdf

   Examples: https://hurel.hanyang.ac.kr/Geant4/Geant4_GPS/reat.space.qinetiq.com/gps/examples/examples.html
