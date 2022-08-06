# eic_endcap_insert

[![DOI](https://zenodo.org/badge/491633889.svg)](https://zenodo.org/badge/latestdoi/491633889)

Getting started
---------------------------------
Enter the EIC container ([installation instructions](https://eicweb.phy.anl.gov/containers/eic_container)) and go to the directory containing your EIC container (if not already there). 

Note that the latest version defines `$EIC_SHELL_PREFIX` instead of the old version's `$ATHENA_PREFIX`. 

## Installation instructions

If you don't have them already, get the latest ip6 files and install them:
```
git clone https://eicweb.phy.anl.gov/EIC/detectors/ip6.git
cd ip6
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX
make install
cd ../..
```
Then clone this repository and install it:
```
git clone https://github.com/rymilton/eic_endcap_insert.git
cd eic_endcap_insert
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX
make install
```
Make sure IP6 and this repository have the same install prefix.

##### NOTE: If you're using an old version of the EIC container, check if `./local/lib` is listed in `$LD_LIBRARY_PATH` (can check with `echo $LD_LIBRARY_PATH`) after entering the container. If it's not present, add it manually with `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./local/lib`. There is also no need to source `/opt/detector/setup.sh`, as was listed in previous readme.

## Editing the simulation
By default, the simulation includes the HCal insert (W/Sc + Steel/Sc), ECal insert (homogeneous W/ScFi material), HCal (ATHENA: 20/3 mm Steel/Sc), ECal (homogeneous W/ScFi material), and the beampipe. To change what detectors are simulated, simply comment out the undesired ones in `endcapP_insert.xml`.

Some simple parameters for the geometry are contained in `compact/configuration_default.xml`. If you want to simulate without a beampipe and hole, replace `<include ref="compact/configuration_default.xml"/>` with `<include ref="compact/configuration_nohole.xml"/>` and comment out `<include ref="ip6/central_beampipe.xml"/>` in `endcapP_insert.xml`.

#### NOTE: If you adjust any compact or src files, you need to `make install` in your build directory before running the simulation.

## Running the simulation
`run_sim_hepmc` generates a HepMC file and feeds it to npsim and DD4hep. The resulting sim file is then sent through Juggler for digitization and reconstruction. The sim and reco files are saved. 

Some basic, adjustable paramaters are listed at the top of `run_sim_hepmc`.

To run the simulation, use `./run_sim_hepmc`

#### NOTE: If you remove any detector components from the simulation, you must also comment out the related lines in `endcapP_insert_reco.py`

## Output
There will be two output files: a sim file and a reco file. The sim file contains the Geant4 level information while the reco file contains the digitized and reconstructed information. Both contain information about the MCParticles. As a collaboration, we typically use the branches ending in HitsReco. 

## Addressing Homogenous ECal
To avoid high memory usage, the ECal and ECal insert use a mixed material of W/Polystyrene where the weight percentages are calculated based on the empirical weight and density of a prototype W/ScFi ECal. To incorporate the sampling fraction of the W/ScFi, a smearing procedure is needed: $E_{tower} = \mathrm{gRandom->Gaus}\left(E_{tower}*.03, \sigma\right)$, where $\sigma = E_{tower}\sqrt{a^2/E_{tower} + b^2}$, $a = 0.1$, and $b = 0.0015$. $\mathrm{gRandom}$ here is ROOT's random generator. This maintains the mean of W/ScFi and reproduces fluctuations with a random Gaussian. See [this presentation](https://github.com/rymilton/eic_endcap_insert/files/9172710/Smearing.of.mixture.structure.for.EMCal.pdf) (especially slide 3) by Zhiwan Xu, et al. for more details.

## Images of simulation geometry
Whole endcap:

<img src="https://user-images.githubusercontent.com/87345122/180581581-c85ece7d-7137-4392-ada6-f52dbaaec1ff.png" width="450"> <img src="https://user-images.githubusercontent.com/87345122/180581647-cc728b5b-3e67-48fc-bd74-570ddc7933d0.png" width="404">

Front of HCal and insert:

<img src="https://user-images.githubusercontent.com/87345122/180581758-455f3b03-5d8b-4bf1-83a3-18e5a30dafdc.png" width="450">

W/Steel insert:

<img src="https://user-images.githubusercontent.com/87345122/180581614-37ec62c5-132e-4979-8864-8f8996bd86f7.png" width="800">


<!-- ## GPS documentation
---------------------------------
If you want to adjust the particle gun, here's the documentation for the general particle source (GPS):

   Manual: https://www.fe.infn.it/u/paterno/Geant4_tutorial/slides_further/GPS/GPS_manual.pdf

   Examples: https://hurel.hanyang.ac.kr/Geant4/Geant4_GPS/reat.space.qinetiq.com/gps/examples/examples.html -->
