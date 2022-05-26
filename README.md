# eic_endcap_insert

Running the standalone simulation
---------------------------------
Enter the EIC container and source the default enviroment. Then do the following:

First, get the ip6 files:
```
git clone https://eicweb.phy.anl.gov/EIC/detectors/ip6.git
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
./run_sim
```
This will run both the detector simulation step and the reconstruction (JUGGLER) step.

