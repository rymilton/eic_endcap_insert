# eic_endcap_insert

Running the standalone simulation
---------------------------------
Enter the EIC container and source the default enviroment. Then do the following:

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$ATHENA_PREFIX -DCMAKE_CXX_STANDARD=17
make install

cd ..
./run_sim
```
To also run the reconstruction software:
```
export JUGGLER_SIM_FILE=endcapP_insert_sim.root JUGGLER_REC_FILE=rec_cal_output.edm4hep.root JUGGLER_N_EVENTS=100
gaudirun.py endcapP_insert_reco.py
```

